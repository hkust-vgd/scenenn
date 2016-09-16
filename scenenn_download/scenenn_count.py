from __future__ import print_function
import sys, os
from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive

"""
Count number of scenes in the Google Drive.
"""

if __name__ == "__main__":

    # Authentication
    gauth = GoogleAuth()
    gauth.LocalWebserverAuth()
    drive = GoogleDrive(gauth)

    # SceneNN public Google Drive folder ID 
    root_id = '0B-aa7y5Ox4eZWE8yMkRkNkU4Tk0'
   
    file_list = drive.ListFile({'q': "'%s' in parents and trashed=false" % root_id}).GetList()
    count = 0
    for file1 in file_list:
        try:
            int(file1['title'])
            count += 1
        except:
            continue
    print("Scenes: %d" % count)
    