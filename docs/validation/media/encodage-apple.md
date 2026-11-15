# Vérification ciblée de l’encodage Apple

Le banc compile 119 sources C# originales et vérifie leurs empreintes contre l’inventaire. Empreinte du manifeste : `553625ded11b3647956b34801117bc0259c06dbb8ff6a537078667d1eb5bb6ad`.

413 cas de référence C# ont été exécutés : empaquetage/dépaquetage MSB, compte positif et dépassement arithmétique, remplissage zéro/un, capacité insuffisante, pistes vides, limites de longueur NIB/WOZ, plus de quarante pistes NIB, sorties compactes FM/MFM, identifiants supportés, géométries et données DOS/ProDOS/RWTS18, secteurs manquants/courts/dupliqués, permutation des adresses physiques, registres injectés ou vides, annulation et taille maximale du résultat. Les charges ProDOS identifient individuellement chaque moitié de bloc : le banc compare les requêtes physiques et les octets des véritables encodeurs.

Le constructeur de catalogue par défaut C# non utilisé par ces comparaisons est un point d’injection du banc ; les cas exécutés injectent les véritables encodeurs Apple ou des encodeurs d’observation dérivés du véritable TrackEncoderBase. Aucun algorithme d’encodage n’est simulé dans les comparaisons de sorties réelles. Le registre par défaut C++ est couvert séparément par un contrôle natif.

Les erreurs sont normalisées en acceptation/refus, sans prétendre identifier les types d’exception .NET et C++. Les sorties binaires complètes sont comparées par SHA-256. Le banc natif prévoit également l’écriture atomique par chemin, l’annulation et la conservation du fichier existant ; les fichiers temporaires sont libérés par RAII.

La première comparaison native donne 412 résultats identiques sur 413. Un écart réel de portage est détecté au compte de bits 2 147 483 640 : la simplification de la borne en `+7` ignorait le débordement intermédiaire de `+8` dans l’expression C# vérifiée. La borne du packer est corrigée. Après reconstruction, les treize cas de comptes et un nouveau cas de borne immédiatement valide (2 147 483 639) donnent **14 résultats identiques, zéro différence**. Les autres parcours réussis ne sont pas rejoués. Les **414 cas distincts** sont ainsi validés, avec quatre contrôles natifs supplémentaires d’écriture, annulation, conservation et registre par défaut réussis.

Les bibliothèques et le banc compilent sans avertissement C++. Les 29 catalogues Argos sont relus et corrigés ; clés et paramètres sont contrôlés. Aucun ancien ensemble de tests inchangé n’est réexécuté : seuls les parcours affectés par le packer partagé sont repris dans ce nouveau banc.

Le banc et ses fichiers temporaires ont été supprimés après intégration et vérification de l’inventaire par l’agent principal. Aucun test ajouté au workflow GitHub.
