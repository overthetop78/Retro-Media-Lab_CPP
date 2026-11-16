# Creative Voice, profil legacy

Référence C# consultée en lecture seule le 16 novembre 2026 : Images/Formats/Tape/Voc/VocReader.cs et VocWriter.cs dans F:/GW GUI/src/RetroMediaLab.MediaEngine. Les SHA-256 correspondent à l’inventaire.

Le reader conserve le profil Caprice32 du C# : signature Creative Voice, offset de données, blocs 1 son PCM non signé 8 bits, 2 continuation, 3 silence, 4 marqueur et 5 texte ; les autres types et les changements de constante de temps sont refusés. Les blocs fixes 3 et 4 n’utilisent pas de longueur 24 bits dans ce profil hérité. Il ne s’agit pas d’une implémentation de tous les blocs VOC possibles. Version et checksum d’entrée restent ignorés comme dans le C#.

Les échantillons sont normalisés en mono signé 16 bits little-endian : (octet−128)×256. La fréquence est arrondie au pair à partir de 1000000/(256−constante). Le document conserve temps, longueur, source PCM possédée, métadonnées et constante d’origine. Un terminateur absent donne un avertissement traduit, tandis qu’un bloc tronqué ou une absence de données est rejeté.

Le writer choisit le premier canal seulement, conserve les segments Samples sélectionnés par le C#, et réutilise l’extraction PCM 8/16/24/32 bits. Il calcule la constante bornée 1–255 puis rééchantillonne par sélection du sample précédent, sans remplacer cet algorithme par une interpolation. Il ne décode pas les canaux inutilisés. Les ranges interrompant une trame sont refusés ; tailles et accumulation sont bornées avant allocation.

L’en-tête écrit la version 0x0114 et son checksum. Le premier bloc accepte 0x00fffffd échantillons (deux octets de paramètres en plus), les blocs suivants 0x00ffffff ; la fin utilise un octet nul. La publication utilise le writer atomique commun ; sources/buffers sont possédés et libérés par RAII sur exception ou annulation.

VOC et Creative Voice sont ajoutés au fichier commun d’invariants, après correction des traductions générées qui altéraient ces identifiants. Le raccordement à la composition globale reste à faire avec les autres familles de bandes.
