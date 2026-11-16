# Conteneur physique MFM EMU

Référence : les dix sources C# `Images/Formats/HardDisk/MfmEmu/`, consultées en lecture seule. Leurs empreintes correspondent à l'inventaire. L'identifiant writer `mfm-emu` et le système de fichiers `acorn-adfs` reprennent les constantes C#.

Le découpage C++ sépare constantes, en-tête possédé, projection temporelle des cellules, conteneur immuable, génération à la demande, représentation et adaptateurs de lecture/écriture/création. Les en-têtes supplémentaires des révisions mineures sont conservés. Les adresses physiques sont validées avant publication ; le marqueur terminal et la longueur déclarée doivent correspondre exactement à la source.

Les cellules sont des mots little-endian, bit 31 en premier. La période de rotation est fournie par l'appelant ; la taille réservée dans le conteneur ne la définit pas. Les fenêtres conservent les cellules non traversées et le remplissage, quantifient les transitions à la cellule suivante et respectent le choix d'inclure l'instant terminal.

La représentation reste de type Flux tout en exposant le contrat de blocs pour le lecteur logique Adaptec existant. Sa vue logique est un document séparé ; aucune impulsion, géométrie ou donnée de secteur n'est inventée pour les pistes vierges. Les changements de pistes créent un nouveau conteneur et un nouveau cache de lecteur. Les callbacks possèdent leurs sources ; verrous et buffers sont libérés par RAII.

La génération formatée conserve un seul cache de piste, l'entrelacement C# et les paramètres initiaux Adaptec. La création ADFS réutilise le formateur MediaFileSystems. L'écriture passe par la transaction commune, accepte la réécriture du fichier source et propose une réservation physique facultative ; la création refuse de remplacer un fichier existant.

Les erreurs utilisent les catalogues MediaEngine. La clé `MediaEngine.MfmEmuHeader` évite de confondre cette erreur avec le message de décodage ISO MFM déjà nommé `MediaEngine.MfmHeader`.
