%extend Linklist
{
  Entry *search(const char* what)
  {
    return self->search((char*)what,0);
  }
}

%extend Layer {
  void add_filter(Entry *filter_entry)
  {
    Filter *filter = (Filter*)filter_entry;
    filter->apply(self);
  }
}

