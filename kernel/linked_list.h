#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

/**
 * Type structure pour une liste chaînée simple
 */
typedef struct simple_link {
	void *next;  /* Pointeur générique vers le prochain élément */
} simple_link;

/**
 * Initialiser une tête de liste (pointeur vers le premier élément)
 */
#define simple_list_init(head) do { *(head) = NULL; } while (0)

/**
 * Tester si une liste est vide
 * head : pointeur vers la tête de liste (pointeur vers le premier élément)
 * retourne 1 si vide, 0 sinon
 */
#define simple_list_empty(head) ((head) == NULL)

/**
 * Ajouter un élément à la fin de la liste (FIFO)
 * ptr_elem  : pointeur vers l'élément à ajouter
 * head      : adresse du pointeur vers la tête de liste
 * type      : type de l'élément
 * field     : nom du champ de type 'simple_link'
 */
#define simple_list_add(ptr_elem, head, type, field)                    \
	do {                                                             \
		type *__elem = (ptr_elem);                               \
		__elem->field.next = NULL;                               \
		if (*(head) == NULL) {                                   \
			*(head) = __elem;                                \
		} else {                                                 \
			type *__cur = *(head);                           \
			while (__cur->field.next != NULL) {              \
				__cur = (type *)__cur->field.next;            \
			}                                                \
			__cur->field.next = __elem;                      \
		}                                                        \
	} while (0)

/**
 * Récupération du pointeur vers l'objet correspondant
 *   (On calcule la différence entre l'adresse d'un élément et l'adresse
 *   de son champ de type 'link' contenant les liens de chainage)
 * ptr_link  : pointeur vers le maillon
 * type      : type de l'élément à récupérer
 * listfield : nom du champ du lien de chainage
 */
#define list_entry(ptr_link, type, listfield) \
	((type *)((char *)(ptr_link)-(unsigned long)(&((type *)0)->listfield)))

/**
 * Parcourir une liste
 * ptr_elem  : pointeur utilisé comme itérateur de boucle
 * head      : tête de liste (pointeur vers le premier élément)
 * type      : type des éléments
 * field     : nom du champ de type 'simple_link'
 */
#define simple_list_for_each(ptr_elem, head, type, field)              \
	for (ptr_elem = (head);                                          \
	     ptr_elem != NULL;                                           \
	     ptr_elem = (type *)ptr_elem->field.next)

/**
 * Enlever le premier élément de la liste
 * head      : adresse du pointeur vers la tête de liste
 * type      : type de l'élément
 * field     : nom du champ de type 'simple_link'
 * retourne un pointeur vers l'élément enlevé, NULL si la liste est vide
 */
#define simple_list_remove_first(head, type, field)                    \
	({                                                               \
		type *__first = NULL;                                    \
		if (*(head) != NULL) {                                   \
			__first = *(head);                               \
			*(head) = (type *)(*(head))->field.next;         \
			__first->field.next = NULL;                      \
		}                                                        \
		__first;                                                 \
	})

/**
 * Enlever un élément spécifique de la liste
 * ptr_elem  : pointeur vers l'élément à enlever
 * head      : adresse du pointeur vers la tête de liste
 * type      : type de l'élément
 * field     : nom du champ de type 'simple_link'
 */
#define simple_list_remove(ptr_elem, head, type, field)               \
	do {                                                             \
		type *__elem = (ptr_elem);                               \
		if (*(head) == __elem) {                                 \
			*(head) = (type *)__elem->field.next;           \
		} else {                                                 \
			type *__cur = *(head);                           \
			while (__cur != NULL && __cur->field.next != __elem) { \
				__cur = (type *)__cur->field.next;            \
			}                                                \
			if (__cur != NULL) {                              \
				__cur->field.next = __elem->field.next;     \
			}                                                \
		}                                                        \
		__elem->field.next = NULL;                               \
	} while (0)

#endif
