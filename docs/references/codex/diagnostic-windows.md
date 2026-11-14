# Diagnostic de l’isolation Windows Codex

Date : 13 novembre 2026.

## Cause constatée

Les commandes échouent pendant la préparation du sandbox, avant le lancement de Git ou de PowerShell. Elles fonctionnent hors sandbox.

Le journal `C:\Users\overt\.codex\.sandbox\sandbox.2026-11-13.log` indique :

```text
deny ACE failed on F:\Retro-Media-Lab_CPP\.codex:
open deny ACL target for update (READ_CONTROL | WRITE_DAC):
Accès refusé. (os error 5)
```

Lecture des permissions Windows :

- Le projet appartient à `GAMER_PC\overt`.
- Le dossier `.codex` appartient à `GAMER_PC\CodexSandboxOffline`.
- Le fichier `.codex/config.toml` appartient à `GAMER_PC\overt`.
- Le compte utilisateur dispose de droits hérités de modification, mais cette autorisation ne comprend pas la modification des permissions (`WRITE_DAC`).

Le refus porte sur la mise à jour des protections de `.codex`, pas sur le contenu TOML. L’origine historique du changement de propriétaire n’est pas établie.

## Correction appliquée et validée

Dans un terminal PowerShell exécuté en administrateur :

```powershell
icacls 'F:\Retro-Media-Lab_CPP\.codex' /setowner 'GAMER_PC\overt'
```

La commande cible uniquement le propriétaire du dossier, sans parcours récursif ni suppression des protections. Relancer ensuite une commande Codex dans le sandbox pour vérifier que sa préparation aboutit. Après application de la commande par l'utilisateur, git status --short --branch a réussi dans le sandbox, sans élévation. La branche main est alignée sur origin/main. Le blocage de démarrage est résolu.

## Référence officielle

[Documentation OpenAI du sandbox Windows](https://learn.chatgpt.com/docs/windows/windows-sandbox), consultée le 13 novembre 2026 : le sandbox historique elevated utilise des comptes dédiés et les permissions du système de fichiers ; les permissions Windows font partie des causes de panne à examiner.
