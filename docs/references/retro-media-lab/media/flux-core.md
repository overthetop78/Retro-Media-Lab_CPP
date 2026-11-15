# Primitives de flux et GCR Commodore partagé

## Sources et organisation

Dix sources fonctionnelles C# sont portées : FluxBitstream, FluxDecodingParameters, FluxRevolution, FluxRevolutionFactory, FluxTimingEstimator, FluxTimingMode, FluxTransitionDecoder, CommodoreGcrCodec, CommodoreGcrChecksum et FluxDecoderConfidence. Elles ont été compilées directement sans modification et leurs SHA-256 correspondent à l’inventaire. Le banc lie aussi les trois dépendances C# d’erreurs/limites/tailles sectorielles nécessaires à la compilation ; cela ne constitue pas leur portage complet.

`MediaEngine/Models/Flux/` possède bits, intervalles, estimation, reconstruction et conversion en révolution. `Codecs/Commodore/` porte la table GCR 4/5 bits et le checksum XOR commun. `Decoding/Confidence` contient les trois calculs communs de confiance. Ces contrats restent privés au moteur ; aucune dépendance directe d’App à une bibliothèque supplémentaire n’est ajoutée.

Les bits sont possédés sous forme d’octets limités à zéro/un, exposés en lecture seule par Bitstream. La durée est finie et supérieure ou égale à un tick. Les intervalles d’une révolution sont des valeurs uint32 possédées et exprimées dans les ticks de leur source. Aucune échelle universelle en nanosecondes n’est imposée à ces modèles. L’extension circulaire est limitée à une copie de la longueur originale ; le retour C++ utilise une valeur possédée.

## Estimation et reconstruction

L’estimation FM utilise le percentile bas des intervalles positifs. NRZI exclut d’abord l’intervalle d’index, avec repli sur tous les intervalles positifs si nécessaire. L’estimation non-FM prend la médiane du premier cinquième des échantillons positifs triés puis la divise par deux. Le repli et le minimum sont un tick.

La reconstruction adaptative quantifie chaque intervalle, conserve la transition finale et ajuste la cellule seulement à partir du deuxième intervalle. L’observation est acceptée entre 0,7 et 1,3 fois la cellule courante, avec coefficient 0,08. La durée retournée est la moyenne des cellules retenues après le premier intervalle ; le mode explicite NRZI sans adaptation conserve la durée initiale. Les variantes FM/MFM utilisent au plus 32 cellules, NRZI au plus 64.

La PLL conserve les constantes et l’ordre des opérations C# : limites 0,9/1,1 du centre, seuil de demi-cellule, rétention de phase 0,4, correction 0,05 et correction directe jusqu’à trois zéros. Les entrées nulles peuvent être accumulées sans produire immédiatement de transition. L’arrondi de la reconstruction adaptative est au pair ; les conversions hors plage suivent la saturation observée du runtime .NET 10 de référence avant la borne de cellules. La première comparaison a révélé une différence de conversion sur les grands intervalles, corrigée dans le C++.

Le comportement particulier de la PLL avec un maximum nul ou négatif est conservé : il ne suit pas le rejet appliqué par Math.Clamp dans la reconstruction adaptative. Les limites réellement utilisées par les codecs restent 32/64. Les bornes et la soustraction entière sont reproduites sans débordement signé C++.

La création de révolution émet un intervalle à chaque bit de transition, plus l’intervalle final optionnel. La durée d’index est conservée ; aucun remplissage jusqu’à celle-ci n’est inventé. Les cellules et la durée d’index doivent être non nulles ; les produits qui dépassent uint32 sont refusés.

## GCR et confiance

La table de 16 symboles produit cinq bits par demi-octet, dix par octet. Encodage, décodage par demi-octet/octet/plage et écriture dans un tampon avec pas sont conservés. Les plages de décodage respectent la borne finale exclusive du C#, même lorsque le dernier bit réellement adressé précède cette borne à cause du pas. Le pas nul est également comparé au C#, y compris les écritures qui échouent faute de position cible. Les positions non écrites d’un tampon agrandi restent à zéro. Le checksum est le XOR de tous les octets, valide lorsqu’il vaut zéro.

Les calculs de confiance conservent le plafond à un sans inventer de plancher à zéro, les valeurs NaN et les infinis. L’arithmétique entière de la pondération reproduit le bouclage int32 du C# sans comportement indéfini C++.

## Robustesse et limites

Les tailles sont contrôlées avant croissance et les calculs de coordonnées sont bornés. Les coordonnées négatives et les dépassements, que certaines méthodes C# laissent atteindre un index ou une allocation invalide, produisent des erreurs codées. Les tableaux/positions restent limités à la représentation int32 des sources. Les cellules non finies/non positives sont refusées de façon contrôlée ; les fonctions à durée explicite conservent leur borne minimale.

Les traitements acceptent un jeton d’annulation et possèdent leurs buffers par RAII. L’annulation pendant une écriture dans un tampon peut conserver les cellules déjà écrites, comme une mutation progressive ; elle ne promet pas une publication atomique de ce tampon. Une annulation déjà demandée est vérifiée avant mutation. Les erreurs de cellule, taille, bit, intervalle et coordonnées ont huit clés traduites dans les 29 langues. La relecture Argos a notamment corrigé la confusion entre le tick temporel et l’insecte, ainsi que plusieurs négations inversées.

Le décodeur/encodeur de piste spécifique au Commodore 900, les structures/secteurs décodés, registres et conteneurs de flux restent à porter. Ce bloc prépare leur véritable intégration ; il ne les remplace pas par une interprétation partielle. [Validation](../../../validation/media/flux-core.md).
