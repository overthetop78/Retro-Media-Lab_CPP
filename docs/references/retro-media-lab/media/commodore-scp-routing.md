# Routage des reconstructions Commodore SCP

Le bloc 26 complète le point d’entrée du lecteur Commodore SCP. `Formats/CommodoreDisks/Router.cpp` choisit uniquement la branche ; les reconstructeurs GCR, 1581 et 900 gardent leurs responsabilités et diagnostics. [Validation](../../../validation/media/commodore-scp-routing.md).

La sélection reproduit les comparaisons exactes C# : `commodore.1581` appelle MFM 1581, `commodore900.coherent` appelle GCR Commodore 900, toute autre valeur appelle GCR 1541/1571 en conservant la demande originale. Il n’y a ni normalisation, ni détection silencieuse d’un autre codec après échec. L’absence de demande conserve la reconstruction GCR automatique.

Deux surcharges synchrones consomment soit une capture SCP déjà lue, soit un chemin. L’entrée fichier réutilise `fileSource` et `Scp::read` ; aucun autre parseur ou modèle de capture n’est ajouté. La capture et la source restent locales ; le résultat possède ses blocs et survit à leur destruction. L’annulation est contrôlée avant toute ouverture, pendant la lecture et dans la reconstruction. Les exceptions de progression sont propagées et les fichiers libérés par RAII, y compris en cas d’échec.

Le C# exposait une enveloppe Task autour du lecteur de fichier ; le point C++ reste synchrone comme les lecteurs existants. Son exécution sur un worker relève de l’orchestration future, sans introduire de dépendance GUI dans MediaEngine. Les registres de formats/documents, conversions et orchestration générale restent à porter.
