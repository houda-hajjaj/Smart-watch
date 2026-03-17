#ifndef CONSOLE_THREAD_H
#define CONSOLE_THREAD_H

/**
 * @brief Lance le thread d'affichage série (dashboard terminal).
 *        Appeler après shared_data_timers_start().
 *        Aucun retour d'erreur : le thread est purement optionnel.
 */
void console_thread_start(void);

#endif /* CONSOLE_THREAD_H */