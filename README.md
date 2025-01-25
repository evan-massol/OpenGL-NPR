# Projet OpenGL : Démonstrateur pédagogique

#### Evan MASSOL

### 1. Objectif

Ce projet conséquent a pour objectif de permettre à n'importe quel utilisateur d'utiliser facilement une interface simple permettant de manipuler des modèles 3D au format `.obj` ainsi que modifier leurs propriétés de rendu au travers d'une fenêtre ImGUI permettant ces modifications. 

Ce projet permet principalement de réaliser un rendu non photoréaliste (Non-Photorealistic  rendering, ou NPR) et contient donc des propriétés qui sont propres à ce type de rendu.

### 2. Technologies utilisées

- `OpenGL` (fichiers de shaders écrits en GLSL).
- `C++`
- `Cmake` pour la compilation de code, couplé à la commande `make`.
- `Glad`
- `GLFW` pour la gestion des fenêtres.
- `GlEngine`
- `ImGUI` pour l'interface graphique.
- `stbimage` pour les chargements d'images appliquées à des textures.

### 3. Paramètres

La dernière version de ce projet contient, entre autres, les paramètres modifiables suivants:

- Voir le simple *mesh* (faces avec arêtes seulement) ou voir le modèle 3D entier avec remplissage des couleurs, entre autres.
- Couleur d'arrière-plan.
- Choix du modèle 3D à charger (3 modèles sont déjà disponibles par défaut).
- Couleur du modèle 3D.
- Couleur du contour du modèle 3D.
- Epaisseur des bords du modèle 3D.
- Les rotations sur les axes X, Y et Z.
- La position et la couleur de la source de lumière.

Concernant les paramètres spécifiques au NPR, il y a:

- Un **seuil de dégradé de couleurs** du modèle 3D (plus il est élevé, moins les nuances de couleurs sont visibles).
- Un **seuil d'intensité des bords** du modèle 3D (plus il est élevé, plus les bords du modèles 3D seront prononcés).
- La **couleur des bords** du modèle 3D.
- Une **valeur de tramage** (représenté sur le modèle 3D par des points de couleur présents tous les `X` pixels, où `X` est la valeur choisie dans l'interface de ImGUI).
- La **couleur de ces points de tramage**.

### 4. Compilation

Dans la mesure où ce projet utilise `cmake` pour la compilation, il est recommandé de créer un dossier à part afin de le compiler à l'intérieur, et éviter de mélanger les fichiers source avec les fichiers de compilation.

Une fois dans le dossier principal du projet, créer un dossier `build`, puis se placer dedans et faire la compilation avec `cmake`:
```sh
#Sous Linux
mkdir build
cd build
cmake ..
```

Cela génèrera tous les fichiers de compilation à l'intérieur du dossier de build.  
Une fois réalisé, utiliser la commande `make` pour générer la solution et créer l'exécutable du projet:  
`make`

Une fois terminé, il suffit d'exécuter la commande suivante, toujours dans le dossier `build`, afin de lancer l'application:  
`./project/project/project`

### 5. Autre contrôles

La bibliothèque `GLFW` permet aussi à l'utilisateur d'avoir d'autres contrôles à sa disposition. On retrouve notamment:
- Le zoom avec la molette de la souris.
- La rotation de la scène 3D en maintenant le clic gauche de la souris et en bougeant.
- Un mouvement seulement en 2D en maintenant le clic droit enfoncé.
