#include "message.h"
#include "processus.h"
#include "mem.h"
#include "stdbool.h"

uint32_t last_fid = 0;
message_t* message_tab[NBQUEUE];

/**
 * La primitive pcreate alloue une file de capacité égale à la valeur de count.
 * S'il n'y a plus de file disponible ou si la valeur de count est négative ou nulle la valeur de retour de pcreate est strictement négative sinon elle identifie la file qui a été allouée.
 *
 * int count: taille maximum de la file à allouer
 * return: l'identifiant de la file nouvellement créée
 */
int pcreate(int count) {
    // Count négatif ou plus de file disponible
    if (count <= 0 || count > MAX_BUFFER) {
        return -1;
    }

    bool space_found = false;
    // Calcul du prochain FID disponible
    for (size_t i = 1; i < NBQUEUE; i++) {
        size_t slot = (last_fid + i) % NBQUEUE;
        if (message_tab[slot] == NULL) {
            last_fid = slot;
            space_found = true;
            break;
        }
    }
    // plus de file dispo
    if (!space_found) { return -1; }

    int *buf = mem_alloc((unsigned long)count * sizeof(int));
    if (!buf) return -1;

    message_t *m = mem_alloc(sizeof(message_t));
    if (!m) {
        mem_free(buf, (unsigned long)count * sizeof(int));
        return -1;
    }

    m->fid = (uint32_t)last_fid;

    m->buffer = buf;
    m->buf_head = 0;
    m->buf_tail = 0;
    m->count = 0;
    m->capacity = (uint32_t)count;

    INIT_LIST_HEAD(&m->sender_queue);
    INIT_LIST_HEAD(&m->receiver_queue);

    message_tab[last_fid] = m;
    return (int)last_fid;
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
    if (m->count == 0 && !queue_empty(&m->receiver_queue)) {
        // Livraison directe au recepteur le plus prioritaire
        processus_t *recv = queue_out(&m->receiver_queue, processus_t, link);
        recv->state = ACTIVABLE;
        recv->message = message;
        queue_add(recv, &queue_process, processus_t, link, prio);
        ordonnance();
    }
    // Si la file est pleine, le processus appelant passe dans l'état bloqué sur file pleine jusqu'à ce qu'une place soit disponible dans la file pour y mettre le message.
    else if (m->count >= m->capacity) {
        actif->state = BLOCK_MSG_SND;
        actif->blocking_fid = fid;
        actif->message = message;
        ordonnance();
        // Apres reveil : verifier si la file a ete supprimee
        if (actif->blocking_fid == -1) return -1;
        // Deposer le message dans le tampon
        m->buffer[m->buf_tail] = message;
        m->buf_tail = (m->buf_tail + 1) % m->capacity;
        m->count++;
    }
    // Sinon, la file n'est pas pleine et aucun processus n'est bloqué en attente de message. Le message est alors déposé directement dans la file.
    else {
        // deposer dans le tampon
        m->buffer[m->buf_tail] = message;
        m->buf_tail = (m->buf_tail + 1) % m->capacity;
        m->count++;
        ordonnance();
    }

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
    if (m->count > 0) {
        int msg = m->buffer[m->buf_head];
        m->buf_head = (m->buf_head + 1) % m->capacity;
        m->count--;
        if (message) *message = msg;

        // Si la file était pleine, il faut alors immédiatement compléter la file avec le message du premier processus bloqué sur file pleine ; ce processus devient activable ou actif selon sa priorité
        if (!queue_empty(&m->sender_queue)) {
            processus_t *sndr = queue_out(&m->sender_queue, processus_t, link);
            sndr->state = ACTIVABLE;
            queue_add(sndr, &queue_process, processus_t, link, prio);
        }
        ordonnance();
    } else {
        // Si aucun message n'est présent, le processus se bloque et sera relancé lors d'un dépôt ultérieur
        actif->state = BLOCK_MSG_RCV;
        actif->blocking_fid = fid;
        ordonnance();
        if (actif->blocking_fid == -1) return -1;
        if (message) *message = actif->message;
    }

    return 0;
}

/**
 * Passe tous les processus_t de la queue head à l'état ACTIVABLE et les rajoutent dans queue_process.
 *
 * link head: la liste à vider et à déplacer dans queue_process
 * return: rien, les processus sont changé d'état et déplacés dans queue_process
 */
void unblock_file_process(link* head) {
    while (!queue_empty(head)) {
        processus_t *proc = queue_out(head, processus_t, link);
        if (!proc) { return; }
        proc->state = ACTIVABLE;
        proc->blocking_fid = -1;
        queue_add(proc, &queue_process, processus_t, link, prio);
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
    mem_free(m->buffer, (unsigned long)m->capacity * sizeof(int));

    // pdelete détruit la file de messages identifiée par fid et fait passer dans l'état activable, ou actif, tous les processus, s'il en existe, qui se trouvaient bloqués sur la file.
    unblock_file_process(&m->sender_queue);
    unblock_file_process(&m->receiver_queue);

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

    processus_t* proc_iter;
    int nb_data = 0;
    if (m->count == 0) {
        // valeur négative égale à l'opposé du nombre de processus bloqués sur file vide
        queue_for_each(proc_iter, &m->receiver_queue, processus_t, link) { nb_data--; }
    } else {
        // valeur positive égale à la somme du nombre de messages dans la file
        nb_data = (int)m->count;

        // valeur positive égale au nombre de processus bloqués sur file pleine
        queue_for_each(proc_iter, &m->sender_queue, processus_t, link) { nb_data++; }
    }

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

    // Vider la file
    m->buf_head = 0;
    m->buf_tail = 0;
    m->count    = 0;

    unblock_file_process(&m->sender_queue);
    unblock_file_process(&m->receiver_queue);

    ordonnance();

    return 0;
}
