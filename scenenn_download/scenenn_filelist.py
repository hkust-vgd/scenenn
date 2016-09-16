from __future__ import print_function
import sys, os
from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive

"""
A simple script to generate a CSV file that lists all links to download the files 
from the public Google Drive. 

This is good for downloading all XMLs at once.

Due to virus scan before downloading at Google Drive, it is not possible to directly download 
the PLY files from the generated links. User has to click confirm in the browser to initiate 
the download. We recommend using scenenn_download.py instead to automate the download.
"""

def list_folder(writer, id, recursive=False):
    file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % id}).GetList()
    for file1 in file_list:
        if recursive and file1['mimeType'] == u'application/vnd.google-apps.folder':
            list_folder(writer, file1['id'], recursive)
        else:
            writer.write('%s, %s\n' % (file1['title'], file1['webContentLink']))
    
    # Leave a blank line after each folder scan
    writer.write('\n')
        
if __name__ == "__main__":

    # Authentication
    gauth = GoogleAuth()
    gauth.LocalWebserverAuth()
    drive = GoogleDrive(gauth)

    # SceneNN public Google Drive folder ID 
    root_id = '0B-aa7y5Ox4eZWE8yMkRkNkU4Tk0'
    
    scene = ''
    if len(sys.argv) >= 2:
        scene = sys.argv[1]
    
    f = open('filelist.csv', 'w')
    f.write('filename, link\n')
    
    if scene == '':
        # List entire folder recursively
        list_folder(f, root_id, True)
    else:
        # List a specific scene
        scene_id = ''
        file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % root_id}).GetList()
        for file1 in file_list:
            if file1['title'] == scene:
                scene_id = file1['id']
                break
        
        if scene_id == '':
            print("Scene %s not found. Exiting..." % scene_id)
            sys.exit(1)
            
        list_folder(f, scene_id, True)
    
    f.close()
    