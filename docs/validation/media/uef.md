# Conteneur UEF

Référence : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Constants/UefConstants.cs` et `Images/Formats/Tape/Uef/{UefFormat,UefReader,UefWriter}.cs`. Sources C# inchangées, SHA vérifiés dans l’inventaire.

Les constantes UEF et la déclaration de format sont portées. Le lecteur accepte les versions jusqu’à 0.10, directement ou via gzip. La reconnaissance ne décompresse que le header nécessaire. La lecture complète borne la taille logique à 256 MiB ; les entrées gzip sont lues par blocs de 64 KiB. Chaque payload reste possédé après retour et chaque chunk reste dans l’ordre source, avec identifiant, contexte fréquence/débit/phase, durée éventuelle et face. Les chunks inconnus restent disponibles et produisent un avertissement traduit. La validité des paramètres de protocole n’est pas inventée : les payloads restent présents même lorsqu’ils ne mettent pas à jour le contexte, comme en C#.

Le writer reprend le profil de compression et de version, valide les identifiants hexadécimaux et les plages stockées, trie les chunks de façon stable par position et écrit leurs octets sans interprétation. Les versions en métadonnées suivent le parsing décimal strict du C#. Le gzip est produit par flux, sans reconstruction d’un conteneur complet en mémoire. Le writer existant possède la publication atomique ; zlib est libéré par RAII sur toutes les sorties.

Compilation Debug réussie sans avertissement après correction de deux comparaisons signé/non signé dans le portage. Contrôle ciblé PASS : aller-retour direct et gzip, chunks vides/inconnus, header/version, contexte fréquence/débit/phase, gap et face, framing tronqué, limites de lecture, paramètres numériques invalides, profil strict, tri stable et égalités de position, signal vide et annulation conservant la destination. Le banc a été corrigé pour ne pas construire une plage hors source que le modèle commun refuse déjà ; ce défaut concernait le banc, pas l’application.

Les diagnostics ont été générés dans 29 langues avec Argos, relus et corrigés (termes techniques, négations, contresens et chinois traditionnel). UEF et gzip sont protégés dans la liste commune. Les 30 DLL de catalogues sont compilées. Audit PASS : 1251 fichiers C++, 56 clés d’erreur App et 29 langues. Les fichiers temporaires du contrôle sont supprimés après consignation.

La synthèse du signal UEF, le protocole Acorn, la composition automatique et les services globaux de conversion restent à porter. Cette validation du conteneur ne prouve pas leur fonctionnement depuis App.
