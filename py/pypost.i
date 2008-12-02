
/* Extensions to the cpp api */
/* dictionary like access to linked list */
%extend BaseLinklist
{
  %pythoncode %{
    def __len__():
        return self.len()
    def __getitem__(self,desc):
        if isinstance(desc,str):
            return self.search(desc)[0]
        else:
            return self.pick(desc)
  %}

}

%extend Blitter
{
   %pythoncode %{
   def _get_zoomx(self):
       return self.zoom_x
   def _get_zoomy(self):
       return self.zoom_y
   def _set_zoomx(self,val):
       self.set_zoom(val,self.zoom_y)
   def _set_zoomy(self,val):
       self.set_zoom(self.zoom_x,val)
   zoomx = property(_get_zoomx,_set_zoomx)
   zoomy = property(_get_zoomy,_set_zoomy)
   %}
}

%extend Parameter
{
   double getDouble()
   {
      return *(double*)self->value;
   }
   void setDouble(double val)
   {
      self->set((void*)&val);
   }
   bool getBool()
   {
      return *(bool*)self->value;
   }
   void setBool(bool val)
   {
      self->set((void*)&val);
   }
   %pythoncode %{
      def setValue(self,val):
          if self.type == 0:
              return self.setBool(val)
          elif self.type == 1:
              return self.setDouble(val)
          elif self.type == 2:
              return self.setColor(val)
          elif self.type == 3:
              return self.setPosition(val)
          elif self.type == 4:
              return self.setString(val)
      def getValue(self):
          if self.type == 0:
              return self.getBool()
          elif self.type == 1:
              return self.getDouble()
          elif self.type == 2:
              return self.getColor()
          elif self.type == 3:
              return self.getPosition()
          elif self.type == 4:
              return self.getString()
      def getPosition(self):
          return [0.0,0.0]
      def setPosition(self,val):
          pass
   %}
}

/* add_filter function in the layers :P */
%extend Layer
{
  int GetWidth()
  {
    return self->geo.w;
  }
  int GetHeight()
  {
    return self->geo.h;
  }
}

