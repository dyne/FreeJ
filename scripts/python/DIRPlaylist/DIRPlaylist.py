#!/usr/bin/python
# by Jaromil
# based on Lluis rss example, with help from Caedes
# GNU GPL v3

"""
Classes to make a continuous video stream playlist from a directory.

The script is  planned to be a useful tool  for online video ftp sites
to do continous stream like a tv channels on the internet.

The script takes a an argument the directory path

"""

import threading
import freej
import time
import sys
import inspect
import os
import string

end_of_video = False;

class NextVideoCB(freej.DumbCall):
  def __init__(self, *args):
      super(NextVideoCB, self).__init__(*args)

  def callback(self):
      global end_of_video
      end_of_video = True;

class DIRPlaylist(object):
    """A DIRPlaylist is a running freej context and a video playlist from a directory."""
    
    def __init__(self, width=320, height=240, dir = "."):
        global end_of_video
        self.width = width
        self.height = height
        self.ctx_thread = None
        
        files = os.listdir(dir)
        if len(files)==0:
            print("error: no files found in dir " + dir)
            exit()
            
        videos = []
        for f in files:
            if( string.find(f,"mp4")>0):
                videos.append(f)
                print("+ " + f)

        if(len(videos)<1):
            print("error: no videos found in dir " + dir)
            exit()

        self.cx = freej.Context()
        self.cx.init(self.width, self.height, 0, 0)
        self.cx.plugger.refresh(self.cx)
        self.ctx_thread = threading.Thread(target = self.cx.start,
                                           name = "freej")
        self.ctx_thread.start()

        current = 0

        callback = NextVideoCB()

        while (not self.cx.quit):

            if(current >= len(videos)): current = 0
        
            video = freej.VideoLayer()
            video.init(self.cx)
            
            video.open(dir + "/" + videos[current])
            video.add_eos_call(callback)
            video.fit()
            video.active = True
            self.cx.add_layer(video)
            video.start()
            while(not end_of_video): time.sleep(5)
            print "end of video"
            video.quit = True
            time.sleep(1)
            self.cx.rem_layer(video)
            current += 1


############### main()

if len(sys.argv) > 1:
    dirplaylist = DIRPlaylist(320,240,sys.argv[1])
else:
    dirplaylist = DIRPlaylist()

