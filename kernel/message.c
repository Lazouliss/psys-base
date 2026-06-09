#include "message.h"
#include "processus.h"
#include "mem.h"

uint32_t last_queue = 0;
message_t* message_tab[NBQUEUE];

// TODO:
int pcreate(int count) {
    // Count négatif ou plus de file disponible
    if (count <= 0 || last_queue >= NBQUEUE) {
        return -1;
    }

    // Calcul du prochain FID disponible
    for (size_t i = 1; i < NBQUEUE; i++)
    {
        if(message_tab[(i+last_queue) % NBQUEUE] == NULL) {
            last_queue = (i+last_queue) % NBQUEUE;
            break;
        }
    }

    return count;   // TODO:
}

/**
 * Parcours la liste des processus bloqués en attente de message d'une file de messages pour récupérer le processus le plus ancien parmis les plus prioritaires
 * 
 * link head: pointeur de la queue dans laquelle on cherche
 * return: le pid du processus le plus prioritaire et le plus ancien ou 0 si la liste est vide.
 */
int get_pid_proc_block_msg(link head) {
    // Queue vide
    if (queue_empty(&head)) { return 0; }

    processus_t* proc_iter;
    processus_t* proc_prev = queue_top(&head, processus_t, link);
    queue_for_each(proc_iter, &head, processus_t, link) {
        if (proc_iter->prio < proc_prev->prio) {
            return proc_prev->pid;
        }
        proc_prev = proc_iter;
    }

    return proc_prev->pid;
}

/**
 * Envoie le message dans la file identifiée par fid.
 * 
 * int fid: identifiant de la file où le message doit être envoyé
 * int message: message à envoyer
 * return: -1 si la valeur de fid est invalide, sinon 0
 */
int psend(int fid, int message) {
    // fid invalide
    if (fid >= NBQUEUE || fid < 0) { return -1; }

    message_t* m = message_tab[fid];
    if(!m) { return -1; }

    // Si la file est vide et que des processus sont bloqués en attente de message, alors le processus le plus ancien dans la file parmi les plus prioritaires est débloqué et reçoit ce message.
    int pid_old_prio = get_pid_proc_block_msg(m->receiver_queue);
    if (queue_empty(&m->receiver_queue) && pid_old_prio) {
        processus_t* proc_blocked = processus_tab[pid_old_prio];
        proc_blocked->state = ACTIVABLE;
        proc_blocked->message = message;

        queue_del(proc_blocked, link);
        queue_add(proc_blocked, &queue_process, processus_t, link, prio);
    }
    // Si la file est pleine, le processus appelant passe dans l'état bloqué sur file pleine jusqu'à ce qu'une place soit disponible dans la file pour y mettre le message.
    else if (m->size_msg_file == m->max_size_msg_file) {
        actif->state = BLOCK_MSG_SND;
        actif->blocking_fid = fid;

        // rendre la main, elle sera récupérée quand il y aura une case de libre dans la file
        ordonnance();

        // si la file a été supprimée, renvoie -1
        if (actif->blocking_fid == -1) { return -1; }

        // envoyer le message
        msg_node_t *node = mem_alloc(sizeof(msg_node_t));
        node->value = message;
        
        simple_list_add(node, &m->msg_file, msg_node_t, link);
    }
    // Sinon, la file n'est pas pleine et aucun processus n'est bloqué en attente de message. Le message est alors déposé directement dans la file.
    else {
        m->size_msg_file++;

        msg_node_t *node = mem_alloc(sizeof(msg_node_t));
        node->value = message;
        
        simple_list_add(node, &m->msg_file, msg_node_t, link);
    }
    
    ordonnance();

    // si la file a été supprimée, renvoie -1
    if (actif->blocking_fid == -1) { return -1; }

    return 0;
}

/**
 * La primitive preceive lit et enlève le premier (plus ancien) message de la file fid. 
 * Le message lu est placé dans *message si message n'est pas nul, sinon il est oublié.
 * 
 * int fid: identifiant de la file où le message doit être placé
 * int* message: emplacement où le message doit être lu
 * return: -1 si la valeur de fid est invalide, sinon 0
 */
int preceive(int fid, int *message) {
    // fid invalide
    if (fid >= NBQUEUE || fid < 0) { return -1; }

    message_t* m = message_tab[fid];
    if(!m) { return -1; }
    
    // Si un message au moins est disponible dans la file, le premier message est transmis au processus. 
    if (m->size_msg_file > 0) {
        // Si la file était pleine, il faut alors immédiatement compléter la file avec le message du premier processus bloqué sur file pleine ; ce processus devient activable ou actif selon sa priorité
        if (m->size_msg_file == m->max_size_msg_file) {
            int pid_old_prio = get_pid_proc_block_msg(m->sender_queue);
            processus_t* proc_blocked = processus_tab[pid_old_prio];
            proc_blocked->state = ACTIVABLE;

            queue_del(proc_blocked, link);
            queue_add(proc_blocked, &queue_process, processus_t, link, prio);
        }
        
        msg_node_t* node = simple_list_remove_first(&m->msg_file, msg_node_t, link);
        if (node) {
            *message = node->value;
        }
        
        mem_free(node, sizeof(msg_node_t));
        
        m->size_msg_file--;
    }
    // Si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur
    else {
        actif->state = BLOCK_MSG_RCV;
        actif->blocking_fid = fid;
        
        // rendre la main, elle sera rendu plus tard au moment où un message sera envoyé
        ordonnance();

        // si la file a été supprimée, renvoie -1
        if (actif->blocking_fid == -1) { return -1; }

        // récupérer le message transmis
        *message = actif->message;
    }

    return 0;
}

/**
 * Passe tous les processus_t de la queue head à l'état ACTIVABLE et les rajoutent dans queue_process.
 * 
 * link head: la liste à vider et à déplacer dans queue_process
 * return: rien, les processus sont changé d'état et déplacés dans queue_process
 */
void unblock_file_process(link head) {
    processus_t* proc_iter;
    processus_t* proc_next;
    queue_for_each(proc_iter, &head, processus_t, link) {
        // Manuellement passer au next link du proc_iter avant de le supprimer, sinon on perd la référence à la queue et on ne peut plus itérer
        proc_next = queue_entry(proc_iter->link.next, processus_t, link);
        
        proc_iter->state = ACTIVABLE;
        // Les processus libérés auront une valeur strictement négative comme retour de psend ou preceive. 
        proc_iter->blocking_fid = -1;

        queue_del(proc_iter, link);
        queue_add(proc_iter, &queue_process, processus_t, link, prio);

        proc_iter = proc_next;
    }
}

/**
 * Détruit la file de messages identifiée par fid et fait passer dans l'état activable, ou actif, tous les processus, s'il en existe, qui se trouvaient bloqués sur la file. 
 * Les processus libérés auront une valeur strictement négative comme retour de psend ou preceive. 
 * Les messages se trouvant dans la file sont abandonnés.
 * 
 * int fid: la file à supprimer
 * return: -1 si la valeur de fid est incorrecte, sinon 0
 */
int pdelete(int fid) {
    // fid invalide
    if (fid >= NBQUEUE || fid < 0) { return -1; }
    
    message_t* m = message_tab[fid];
    if(!m) { return -1; }
    
    // Les messages se trouvant dans la file sont abandonnés.
    if (!simple_list_empty(m->msg_file)) {
        msg_node_t* node;
        while (m->msg_file) {
            node = simple_list_remove_first(&m->msg_file, msg_node_t, link);
            mem_free(node, sizeof(msg_node_t));
        }
    }
    
    // pdelete détruit la file de messages identifiée par fid et fait passer dans l'état activable, ou actif, tous les processus, s'il en existe, qui se trouvaient bloqués sur la file. 
    unblock_file_process(m->sender_queue);
    unblock_file_process(m->receiver_queue);

    mem_free(m, sizeof(message_t));

    message_tab[fid] = NULL;
    
    ordonnance();
    
    return 0;
}

/**
 * La primitive pcount lit la quantité de données et de processus en attente sur la file fid. 
 * count n'est pas censé être nul
 * 
 * int fid: la file dont on cherche à compter le nombre de données
 * int count[static 1]: emplacement de la valeur de retour
 * return: -1 si la valeur de fid est incorrecte, sinon 0
 */
int pcount(int fid, int count[static 1]) {
    // fid invalide
    if (fid >= NBQUEUE || fid < 0) { return -1; }
    
    message_t* m = message_tab[fid];
    if(!m) { return -1; }

    // count est invalide
    if (!count) { return -1; }

    // valeur positive égale à la somme du nombre de messages dans la file
    int nb_data = m->size_msg_file;

    processus_t* proc_iter;
    // valeur positive égale au nombre de processus bloqués sur file pleine
    queue_for_each(proc_iter, &m->sender_queue, processus_t, link) { nb_data++; }
    // valeur négative égale à l'opposé du nombre de processus bloqués sur file vide
    queue_for_each(proc_iter, &m->receiver_queue, processus_t, link) { nb_data--; }

    *count = nb_data;

    return 0;
}

/**
 * Vide la file identifiée par la valeur de fid et fait passer dans l'état activable ou actif (selon les priorités) tous les processus, s'il en existe, se trouvant dans l'état bloqué sur file pleine ou dans l'état bloqué sur file vide (ces processus auront une valeur strictement négative comme valeur de retour de psend ou preceive). 
 * Les messages se trouvant dans la file sont abandonnés.
 * 
 * int fid: la file à réinitialiser
 * return: -1 si la valeur de fid est incorrecte, sinon 0
 */
int preset(int fid) {
    // fid invalide
    if (fid >= NBQUEUE || fid < 0) { return -1; }
    
    message_t* m = message_tab[fid];
    if(!m) { return -1; }
    
    // Les messages se trouvant dans la file sont abandonnés.
    if (!simple_list_empty(m->msg_file)) {
        msg_node_t* node;
        while (m->msg_file) {
            node = simple_list_remove_first(&m->msg_file, msg_node_t, link);
            mem_free(node, sizeof(msg_node_t));
        }
    }
    
    // pdelete détruit la file de messages identifiée par fid et fait passer dans l'état activable, ou actif, tous les processus, s'il en existe, qui se trouvaient bloqués sur la file. 
    unblock_file_process(m->sender_queue);
    unblock_file_process(m->receiver_queue);
    
    ordonnance();
    
    return 0;
}
