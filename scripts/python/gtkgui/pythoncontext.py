import sys
import traceback

class PythonExecutionContext(object):
    """
    A class representing an independent execution context.
    It can store global symbols for the scripts executed herein, and
    has functions to run and evaluate python code.
    """
    def __init__(self):
        self._exec_dict = {}

    def __setitem__(self,name,value):
        """
        Set a symbol into the execution context dictionary.
        """
        self._exec_dict[name] = value

    def __getitem__(self,name):
        """
        Get a symbol from the execution context dictionary.
        """
        return self._exec_dict[name]

    def __contains__(self,name):
        """
        See if the execution context holds some symbol.
        """
        return name in self._exec_dict

    def RunFromFile(self,filename):
        """
        Run a script into this execution context directly from a file.
        """
        def printerr(error):
            print " * "+filename+": "+error
        f = open(filename,"r")
        self.RunUserCode(f.read(),printerr)
        f.close()

    def RunUserCode(self, script, printfunc,mode="single"):
        """
        Run some text (python code) into this execution context.
        """
        assigned_object = None # if we make a print we also return the printed eval
        # Try and run the user entered line(s)
        try:
                if script.startswith("print"):
                     try:
                         assigned_object = eval(script[5:].strip(),self._exec_dict)
                     except:
                         traceback.print_exc()
                exec(script,self._exec_dict)
        except: # Prints the REAL exception.
                error = '%s:  %s' % (sys.exc_type.__name__, sys.exc_value)
                for errorLine in error.split('\n'):
                        printfunc(errorLine+"\n") # new line to type into
                #traceback.print_exc()
        return assigned_object

