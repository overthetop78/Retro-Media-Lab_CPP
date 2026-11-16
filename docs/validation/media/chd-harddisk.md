# Validation HDD CHD

Le 16 novembre 2026, compilation Debug de MediaEngine réussie sans avertissement observé (`build/chd-harddisk-build.log`). Résultat du programme natif ciblé : `PASS HDD CHD V5 geometry/SHA1/padding/zero-map/12GiB-sparse/reader-registry/profile-rejections/atomic-cancellation/overwrite` (`build/chd-harddisk-smoke.log`).

Le cas principal contient 1025 secteurs de 512 octets : deux hunks physiques dont le dernier partiel. La géométrie GDDD, les offsets, les propriétés, le remplissage nul et les données relues sont vérifiés. Le SHA1 340ea6854f470da46353fd2de6c507efe6499e3e est calculé indépendamment avec Python hashlib sur le motif logique, puis comparé aux octets d’en-tête. Le conteneur commun relit également le second hunk.

Une entrée de carte nulle expose une plage zéro ; une entrée hors fichier est rejetée à la lecture du document. Les codecs compressés, parent, défaut d’alignement, géométrie invalide/absente et cycle de métadonnées sont refusés. Une fixture de 512 octets représente 12 GiB logiques, avec des hunks de 512 MiB et une carte zéro : elle vérifie les limites propres au lecteur HDD et la lecture du dernier secteur sans créer de gros média.

Le registre sélectionne ce lecteur ; un format explicitement incompatible est refusé. L’annulation laisse la destination préexistante intacte. L’écrasement atomique d’un CHD dont les données servent de source restitue les mêmes octets. Programme et artefacts synthétiques sont supprimés après consignation ; aucun média utilisateur ni ancienne batterie de tests utilisé.

Les cinq diagnostics sont générés avec Argos dans les 29 langues, relus et corrigés, en particulier hunk, GDDD, compression et limites du profil. Les 30 DLL de catalogues compilent (`build/chd-harddisk-catalogs.log`). Audit réussi sur 1126 fichiers (`build/chd-harddisk-audit.log`). Le lecteur CHD optique et la composition globale restent à faire.
