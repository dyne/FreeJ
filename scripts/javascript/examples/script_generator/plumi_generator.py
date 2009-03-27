#/usr/bin/python2.5
# andy@engagemedia.org
#generate a FreeJ javascript file, using a RSS2 feed

# from elementtree import ElementTree as et
from xml.etree import ElementTree as et
from Cheetah.Template import Template
import urllib
import os
import shutil

RSS_FILE="http://www.engagemedia.org/featured-videos/RSS2"
JAVASCRIPT_TEMPLATE="plumi_template.js"
JAVASCRIPT_FILE="plumi.js"

#download latest RSS2 
print "fetching %s " % RSS_FILE
feed = urllib.urlopen(RSS_FILE)
print "parsing %s " % RSS_FILE
parsed_feed = et.parse(feed)

#setup the parameters from channel
print "making js file %s " % JAVASCRIPT_FILE
data_map = {}

#download image, and remember path
image_url = parsed_feed.find('channel').find('image').find('url').text
#get the image
print "fetching image logo %s " % image_url
downloaded_image=urllib.urlretrieve(image_url)
#save it somewhere sensible.
image_name=os.path.basename(image_url)
shutil.copyfile(downloaded_image[0],image_name)
#the abs file path
image_file_path=os.path.abspath(image_name)

data_map['image_file_path'] = image_file_path


data_map['channel_title'] = parsed_feed.find('channel').find('title').text
data_map['videos'] = []

for video_elem in parsed_feed.getiterator('enclosure'):
    video_to_play = video_elem.get('url')
    js_var_name = "video_%s" % (len(data_map['videos']))
    video_dict = {'url':video_to_play, 'js_var_name':js_var_name}
    data_map['videos'].append(video_dict)

#write the new javascript file.
t = Template(file=JAVASCRIPT_TEMPLATE, searchList=[data_map])
f=open(JAVASCRIPT_FILE, 'w')
f.write(t.respond())
f.close()
