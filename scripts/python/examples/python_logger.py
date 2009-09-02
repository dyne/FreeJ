import freej

class MyLogger(freej.WrapperLogger):
  """ This is a basic example for a python logger.

  """

  def __init__(self, *args):
    super(MyLogger, self).__init__(*args)

  def logmsg(self, level, msg):
    """ Override this method to handle log messages
    
    """
    if level == freej.ERROR:
      print "ERROR from python: %s" % msg
    else:
      print "I will just print: %s" % msg


m = MyLogger()

# Register in GlobalLogger to have all messages redirected to your logger (you
# can also register to an instance of a Loggable subclass to receive only
# messages from that object).
freej.GlobalLogger.register_logger(m)

# Set the maximum loglevel GlobalLogger should print/redirect
freej.GlobalLogger.set_loglevel(freej.DEBUG)

