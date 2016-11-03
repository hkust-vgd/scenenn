# SceneNN
We maintain the following repositories on Google Drive for our dataset:

## 1. Main
https://drive.google.com/drive/folders/0B-aa7y5Ox4eZWE8yMkRkNkU4Tk0

This includes all 90+ scenes captured by Asus Xtion PRO. 

To download the scenes from this repository, use *scenenn_download*, a Python script that can download the entire dataset or a few specific scenes. To run this script, please install [PyDrive](https://pypi.python.org/pypi/PyDrive). 

## 2. Beta
https://drive.google.com/drive/folders/0B2BQi-ql8CzeUkJNbEVEQkozSmc

This includes annotation for scenes that might receives some experimental updates, e.g., object poses and better floor aligned meshes. Such annotation will be merged to the main repository once they are stable. 

This repository also includes scenes captured by both Asus Xtion PRO and Kinect v2. 

Please refer to our project page at http://www.scenenn.net for the papers and scene previews in the supplementary document. 

## Folder structure 
Each scene is identified by an ID with three digits, e.g., 076, 123. The data of the scene is structured as follows: 

```
SceneNN
├── 123
│   ├── 123.ply                 /* the reconstructed triangle mesh  */
│   ├── 123.xml                 /* the annotation                   */
│   ├── trajectory.log          /* camera pose (local to world)     */
└── oni
    └── 123.oni                 /* the raw RGBD video               */
```

Depending on which application you are working for, you might want to download only the mesh with annotation (~100 - 200MB per scene), or include the raw RGBD (~1 GB per scene). 
The object tag and axis-aligned bounding box are stored separately in the XML file. If you need object poses, please find them in the `beta` repository. 
We do not provide per-pixel label, as it takes up too much space. You can generate per-pixel label using our annotation tool. 

## Raw RGB-D videos 
We also provide raw RGB-D files that are output from the Asus Xtion camera. They are stored in the `oni` folder in both repositories. (You might need to scroll down to the bottom of the scene list to see it.) 

In order to extract depth and color images from ONI files, please use the tool in the `playback` folder from this repository. 
A precompiled binary for Windows is provided. The source code of this tool is `PlaybackSync.cpp` which is also available in the same folder. 