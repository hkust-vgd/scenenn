# SceneNN
[SceneNN](http://scenenn.net) is an RGB-D dataset with more than 100 indoor scenes. All scenes are carefully annotated with instance-level segmentation. 
![](images/teaser.png?raw=true)

Please refer to our project page at http://www.scenenn.net for the papers and scene previews in the supplementary document. 

To access the dataset, please use following repositories on Google Drive:

## 1. Instance-level segmentation
https://drive.google.com/drive/folders/0B-aa7y5Ox4eZWE8yMkRkNkU4Tk0

This includes all 90+ scenes captured by Asus Xtion PRO. 

Please use the Python script [scenenn_download](download/scenenn_download.py) to download a few specific scenes or the entire dataset. To run this script, please install [PyDrive](https://pypi.python.org/pypi/PyDrive). 

### 3D annotation
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

Depending on which application you are working on, you might want to download only the mesh with annotation (~100 - 200MB per scene), or include the raw RGBD (~1 GB per scene). 
The object tag and axis-aligned bounding box are stored separately in the XML file. If you need object poses, please find them in the `beta` repository, below. 

### 2D annotation 
Please download them here: https://drive.google.com/drive/folders/0B2BQi-ql8CzeaDNQQXNmZHdnSFE

We encode the label to color channels in RGB format. To decode, please use this [simple C++ code](https://github.com/scenenn/shrec17/tree/master/mask_from_label). 

### Raw RGB-D videos 
We also provide raw RGB-D files that are output from the Asus Xtion camera. They are stored in the `oni` folder in both repositories. (You might need to scroll down to the bottom of the scene list to see it.) 

In order to extract depth and color images from ONI files, please use the tool in the `playback` folder from this repository. 
A precompiled binary for Windows is provided. Note that OpenNI 1.5.4 x64 must be installed.  

## 2. Category-level segmentation (beta)
https://drive.google.com/drive/folders/0B2BQi-ql8CzeUkJNbEVEQkozSmc

This includes our experimental category-level segmentation and object pose annotation: 

* Semantic labels using the 40 classes defined in NYU-D v2.
* Object poses and better floor aligned meshes. 

Such annotation will be merged to the main repository once they are stable. 

This repository also includes scenes captured by both Asus Xtion PRO and Kinect v2. 

# ObjectNN 
ObjectNN is a subset of SceneNN that has all RGB-D objects extracted and split into 20 categories. We used it for our SHREC 2017 track: RGB-D to CAD object retrieval. 
For more information, please see [here](http://people.sutd.edu.sg/~saikit/projects/sceneNN/shrec17/index.html). 
![](images/objects.png?raw=true)

# License 
Our datasets are free for educational and research use. We would love to hear from you if you find them useful. If you use the data, please cite
```
@inproceedings{scenenn-3dv16,
    author = {Binh-Son Hua and Quang-Hieu Pham and Duc Thanh Nguyen and Minh-Khoi Tran and Lap-Fai Yu and Sai-Kit Yeung},
    title = {SceneNN: A Scene Meshes Dataset with aNNotations},
    booktitle = {International Conference on 3D Vision (3DV)},
    year = {2016}
}
```

If you intend to use the data for commercial applications, please kindly contact us beforehand at *scenenn[at]gmail.com*. 

