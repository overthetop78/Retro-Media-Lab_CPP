# Processus des outils Greaseweazle

Sources primaires consultées le 14 novembre 2026 :

- [Qt QProcess](https://doc.qt.io/qt-6/qprocess.html) : exécution programme/arguments séparés, flux redirigés, signaux asynchrones, erreur de démarrage, arrêt et modificateur de création Windows. L’application n’exécute pas la chaîne affichée dans un shell.
- [Microsoft Job Objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects) : gestion collective d’un processus et de ses descendants, héritage du job par les enfants et destruction à la fermeture du dernier handle avec JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE.
- [Microsoft UpdateProcThreadAttribute](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute) : PROC_THREAD_ATTRIBUTE_JOB_LIST permet d’associer le processus au job dès sa création. L’implémentation Windows utilise STARTUPINFOEX et EXTENDED_STARTUPINFO_PRESENT, sans fenêtre console.

Le job est possédé par RAII. En fin d’opération, après le délai d’arrêt ou lors de la destruction du service, sa fermeture détruit aussi les descendants encore actifs. La création du job et de sa liste d’attributs doit également nettoyer les acquisitions partielles en cas d’exception. La branche Unix prépare une nouvelle session de processus ; elle n’a pas été exécutée dans cet environnement Windows.

Les sorties UTF-8 sont décodées en conservant l’état entre blocs. Le résultat doit être matérialisé en QString avant la destruction du QByteArray temporaire fourni au décodeur.
