
#ifndef _MGJS_HH_
#define _MGJS_HH_

#include <stdexcept>
#include <node.h>



namespace mgjs {

  class MgException : public std::runtime_error
  {
    /// \brief Constructor
    /// param[in] m the exception message 
    public: MgException(const char*m) :std::runtime_error(m){}
  };


  // inter thread communication data
  class JsCallbackData {
    public: v8::Persistent<v8::Function> cb;
    // public: v8::Arguments args;
  };

  class Op0CallBackData : public JsCallbackData {
    public: std::string config;
    public: std::string stlPath;
  };

  class ProtoJs : public node::ObjectWrap
  {
     private: ProtoJs();

     private: virtual ~ProtoJs();     

     public: static void Init(v8::Handle<v8::Object> exports);

     private: static v8::Handle<v8::Value> New(const v8::Arguments& args);

     private: static v8::Handle<v8::Value>
        Op0(const v8::Arguments& args);

     private: static void Op0Async(uv_work_t *req);

     private: static void Op0Return(uv_work_t *req);

     private: static v8::Handle<v8::Value>
        Op1(const v8::Arguments& args);

     
  };


}

#endif
