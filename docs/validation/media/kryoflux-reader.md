# Validation ciblée KryoFlux

Le 16 novembre 2026 : compilation Debug MediaEngine avec MSVC 19.44 réussie (`build/kryoflux-reader-build.log`), puis compilation incrémentale finale après extraction d'une constante (`build/kryoflux-reader-final-build.log`). Les 30 DLL Constants/langues sont construites (`build/kryoflux-reader-catalogs.log`).

Un programme natif temporaire a vérifié les opcodes courts/directs/longs, NOP1/2/3, débordement 16 bits, blocs d'index et fin, remplissage final, métadonnées dupliquées, première valeur conservée et dernière horloge utilisée. Il a vérifié les intervalles [14,300,65792], durée 66106 ns, le repli sans index et les demi-unités arrondies vers l'extérieur.

Il a également vérifié les rejets pour troncature, index incomplet, position incohérente, horloge nulle, intervalle nul, intervalle/durée hors plage et piste vide ; puis l'annulation. L'intégration couvre deux pistes au préfixe Unicode, l'exclusion d'un autre préfixe, la reconnaissance, le filtre de format explicite, le nom invalide, la protection, les fichiers associés et leur longueur totale.

Résultat : `PASS KryoFlux opcodes/indexes/clock/metadata/rounding/bounds/multitrack/recognition/cancellation`, dans `build/kryoflux-reader-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs n'ont été utilisés. Le programme, ses données synthétiques et ses binaires sont supprimés après consignation.

Les dix diagnostics sont générés par Argos dans les 29 langues, relus et corrigés, notamment les négations, troncatures et rotations physiques mal traduites en révolutions politiques. NN.S.raw est protégé comme invariant commun. Audit statique réussi sur 1028 sources C++ et 29 langues (`build/kryoflux-reader-audit.log`). Ce contrôle ne prouve pas l'intégration globale depuis App, ni le fonctionnement sur Linux/macOS.
