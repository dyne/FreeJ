module Freej
  class Context
    #render once
    def step
      self.cafudda(1)
      self
    end

    #step endlessly, blocking
    def start
      loop { self.step; sleep(0.04) }
    end

    #create a ruby thread and call start in that thread
    def start_threaded
      @thread = Thread.new { self.start }
      self
    end

    #stop the rendering thread if it exists
    def stop
      if @thread
        @thread.terminate
      end
      self
    end
  end

  class BaseLinklist
    #get an item from a linked list by key or index 
    def [](item)
      if item.is_a?(String)
        res = self.search(item)
        if res == 0
          return nil
        else
          return res[0]
        end
      else
        return self.pick(item)
      end
    end
    #iterate through a linked list
    def each
      index = 1
      item = self.pick(index)
      while item != nil
        yield item
        index = index + 1
        item = self.pick(index)
      end
      self
    end
  end
end
