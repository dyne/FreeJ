
/* Extensions to the cpp api */
/* dictionary like access to linked list */
%extend Linklist
{
  Entry *__getitem__(const char *desc)
  {
    return self->search(const_cast<char*>(desc),0);
  }
  Entry *__getitem__(int pos)
  {
    return self->pick(pos);
  }

  int __len__()
  {
    return self->len();
  }

}

/* add_filter function in the layers :P */
%extend Layer
{
  void add_filter(Filter *filter)
  {
    filter->apply(self);
  }
}

