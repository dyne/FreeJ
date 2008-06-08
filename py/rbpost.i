%extend Layer {
  void add_filter(Entry *filter_entry)
  {
    Filter *filter = (Filter*)filter_entry;
    filter->apply(self);
  }
}

