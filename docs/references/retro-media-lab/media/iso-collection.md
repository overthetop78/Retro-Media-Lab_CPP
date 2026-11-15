# Collecte ISO — référence C#

Le bloc 31 reprend le traitement des pistes protégées de IsoScpCandidateDecoder, son score et ses constantes. Le dépôt C# reste en lecture seule. Les 35 sources compilées par le banc correspondent aux empreintes de l’inventaire.

## Sélection par fenêtre

Pour chaque piste, toutes les fenêtres sont construites avant le décodage, afin de conserver les refus précoces des jointures SCP. Chaque fenêtre appelle les identifiants de décodeur dans l’ordre demandé. Le résultat est sélectionné selon `secteurs valides × 1000 + données présentes × 10 − secteurs invalides × 1000 + confiance`. Les opérations entières conservent le comportement int32 source sans débordement signé indéfini en C++. Les valeurs NaN sont classées après les nombres ; les égalités complètes conservent le premier décodeur.

Les données présentes mais vides participent au score et à la collecte. Les secteurs sans données ou de numéro négatif sont ignorés. Le groupe physique utilise le cylindre et la face de la piste ; le groupe adressé ne reçoit le secteur que si son cylindre et sa face enregistrés correspondent. Toutes les métadonnées du secteur et sa révolution à base un sont conservées ; SourceTrack reste absent.

## Parallélisme et données possédées

Le traitement est borné au nombre de pistes et au nombre de processeurs disponibles. Chaque résultat appartient à sa piste. Les groupes sont fusionnés dans l’ordre de l’image protégée (cylindre/face), puis dans leur ordre d’insertion, de révolution et de secteur. La planification des threads ne change pas l’ordre des candidats.

La progression publie chaque révolution décodée et chaque piste terminée avec compteurs atomiques et coordonnées. Elle transporte des champs structurés ; le GUI pourra les présenter et les traduire sans texte métier brut. Les callbacks s’exécutent depuis les workers et doivent synchroniser leur état.

Les jthreads sont joints avant le retour ou la propagation d’une erreur. La première exception enregistrée est propagée et demande l’arrêt des autres workers ; l’annulation initiale et en cours utilise le contrat commun. Les groupes possèdent leurs copies, sans dépendance à la durée de vie des décodeurs ou conteneurs source.

## Contrats et portée

Collector.h expose un appel de décodage fourni par le service appelant, correspondant à decoders.Decode dans le C#. Aucun choix implicite de codecs n’est ajouté. Une liste vide échoue seulement lorsqu’une fenêtre doit sélectionner un résultat ; une piste constituée uniquement de bits ne lance pas de décodeur. Le C++ signale aussi explicitement une fonction absente. Les deux diagnostics de MediaEngine/iso-collection.json sont traduits avec Argos puis relus dans les 29 langues.

Le cache des captures, l’entrée fichier et le registre général des décodeurs restent à porter. Le fichier C# IsoScpCandidateDecoder est donc conservé en état partiel dans l’inventaire. Les autres étapes de progression de l’exploration SCP ne sont pas déclarées portées par ce contrat de collecte.
