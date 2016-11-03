from __future__ import print_function
import sys, os
from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive

"""
A simple script to download the dataset from the paper:
    SceneNN: A Scene Meshes Dataset with aNNotations 
at International Conference on 3D Vision 2016.

# Dependencies
This script depends on PyDrive to talk to Google Drive. 

To install PyDrive, execute 
    pip install PyDrive

# Usage
    python scenenn_download.py  [scene IDs]

At the first time this script is called, your browser window
will show a page with authentication request to access your 
Google Drive. Please allow it. 
This is due to the mechanism behind PyDrive. 
Our script does not touch any of your Google Drive data. 

If the scene ID is not provided, the entire dataset folder 
will be downloaded (~100 GB). 
"""

def list_folder(id, recursive=False):
    file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % id}).GetList()
    for file1 in file_list:
        if recursive and file1['mimeType'] == u'application/vnd.google-apps.folder':
            print('Go to folder: %s' % (file1['title']))
            list_folder(file1['id'], recursive)
        else:
            print('File: %s, id: %s' % (file1['title'], file1['id']))
            

def download_folder(id, local_folder, recursive=False):
    file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % id}).GetList()
    for file1 in file_list:
        if recursive and file1['mimeType'] == u'application/vnd.google-apps.folder':
            print('Go to folder: %s' % (file1['title']))
            child_folder = local_folder + '/' + file1['title']
            mkdir(child_folder)
            download_folder(file1['id'], child_folder, recursive)
        else:
            print('Downloading: %s (%.2f MB) ... ' % (file1['title'], float(file1['fileSize']) / (1024 * 1024)) , end="")
            file1.GetContentFile(local_folder + '/' + file1['title'])
            print('OK')

def mkdir(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)
        
if __name__ == "__main__":

    # Authentication
    gauth = GoogleAuth()
    gauth.LocalWebserverAuth()
    drive = GoogleDrive(gauth)

    # SceneNN public Google Drive folder ID 
    root_id = '0B-aa7y5Ox4eZWE8yMkRkNkU4Tk0'
    
    # Default local folder for downloaded data
    folder = 'scenenn_data'
    mkdir(folder)
    
    scenes = []
    if len(sys.argv) >= 2:
        scenes = sys.argv[1:]
        
    if scenes == []:
        # Download entire folder
        download_folder(root_id, folder, True)
    else:
        for scene in scenes:
            # Download a specific scene
            scene_id = ''
            scene_folder = folder + '/' + scene
            file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % root_id}).GetList()
            for file1 in file_list:
                if file1['title'] == scene:
                    scene_id = file1['id']
                    break
            
            if scene_id == '':
                print("Scene %s not found." % scene_id)
                continue
                
            mkdir(scene_folder)
            download_folder(scene_id, scene_folder, True)
        