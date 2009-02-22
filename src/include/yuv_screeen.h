YuvScreen() : public ViewPort() {
  
 public:
  YuvScreen();
  ~YuvScreen();

  bool init(int w, int h);

  struct SwsContext *sws_ctx;
  AVFrame *yuv_frame;
  AVFrame *rgb_frame;

  AVStream *avstream;

};

