# Conteneur et encodeur MSX CAS

Références C# inchangées : `Constants/MsxCasConstants.cs`, `Images/Formats/Tape/MsxCas/{MsxCasFormat,MsxCasReader,MsxCasWriter}.cs` et `Images/Writing/Encoding/Sequential/Msx/MsxTapeEncoder.cs` dans `F:/GW GUI`. SHA vérifiés dans l’inventaire.

Le lecteur recherche les séparateurs avec état conservé entre tampons de 64 KiB, en accédant directement à la source possédée afin de ne pas remplir le cache de reconnaissance avec le fichier entier. La reconnaissance s’arrête au premier séparateur : en automatique, il doit commencer à zéro et être suivi d’un marqueur répété binaire/BASIC/ASCII ; en explicite, un séparateur complet suffit. Le cas d’un marqueur trop court renvoie false, au lieu de tenter une lecture au-delà du fichier comme pouvait le faire le C#.

La représentation conserve préfixe inconnu, séparateurs et groupes d’octets dans l’ordre, avec leurs plages stockées et les avertissements traduisibles. Aucun timing n’est reconstruit dans le conteneur : la métadonnée reste not-stored et les durées absentes. Le writer trie de façon stable et publie atomiquement les séparateurs et les octets de chaque plage ; les groupes non représentables sont refusés.

L’encodeur séquentiel accepte les groupes validés et produit CAS ou PCM FSK 8N2 à 1200 bauds. Les groupes CAS conservent leur type, et la reconstruction est signalée. Le PCM reprend les silences/porteuses longs du premier groupe ou d’un groupe typé, courts autrement. Sa borne est vérifiée avant allocation ; les octets ne sont pas interprétés comme un système de fichiers.

Compilation Debug réussie sans avertissement. Contrôle ciblé PASS : trois marqueurs, explicite/automatique, séparateur sans payload, séparateur coupé à 64 KiB, préfixes et groupes inconnus, séparateurs vides successifs, aller-retour binaire exact, tri et refus des pertes. Une source instrumentée confirme des lectures de 64 KiB au maximum et l’arrêt précoce de reconnaissance. L’encodage via registre CAS et PCM a été vérifié avec les durées longues/courtes et les groupes retrouvés dans le vrai décodeur FSK 8N2 partagé. Profil, borne et annulation avec destination conservée vérifiés. Aucun média utilisateur ; banc temporaire et artefacts supprimés après consignation.

Les six diagnostics sont générés avec Argos dans les 29 langues, relus/corrigés, puis compilés dans les 30 DLL de catalogues. Audit PASS : 1286 fichiers C++, 56 clés d’erreur App et 29 langues.

Le décodeur MSX (qui dépend aussi de l’expansion TSX), les conteneurs TZX/TSX, la composition automatique et les services globaux restent à porter. Le fichier MsxTapeDecoder n’est pas marqué porté ; ce contrôle du codec FSK ne vaut pas validation de ce décodeur ni de son raccordement à App.
