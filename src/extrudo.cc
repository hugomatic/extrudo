#include "extrudo.h"


#include <sstream>
#include <iostream>

#include <unistd.h> // sleep

#include "mgl/abstractable.h"
#include "mgl/configuration.h"
#include "mgl/miracle.h"

#include "libthing/Vector2.h"

using namespace std;
using namespace v8;
using namespace mgjs;
using namespace mgl;

/////////////////////////////////////////////////
void InitAll(Handle<Object> exports)
{
  ProtoJs::Init(exports);
}

/////////////////////////////////////////////////
NODE_MODULE(extrudo_cc, InitAll)


/////////////////////////////////////////////////
void ProtoJs::Init(Handle<Object> exports)
{
  // Prepare constructor template
  Local<FunctionTemplate> tp1 = FunctionTemplate::New(New);
  tp1->SetClassName(String::NewSymbol("Slicer"));
  tp1->InstanceTemplate()->SetInternalFieldCount(1);

  tp1->PrototypeTemplate()->Set(String::NewSymbol("op0"),
      FunctionTemplate::New(Op0)->GetFunction());

  tp1->PrototypeTemplate()->Set(String::NewSymbol("op1"),
      FunctionTemplate::New(Op1)->GetFunction());


  Persistent<Function> constructor1 =
      Persistent<Function>::New(tp1->GetFunction());

  exports->Set(String::NewSymbol("Slicer"), constructor1);
}


/////////////////////////////////////////////////
Handle<Value> ProtoJs::New(const Arguments& args)
{
  HandleScope scope;

  ProtoJs* obj = new ProtoJs();
  obj->Wrap(args.This());

  return args.This();
}

/////////////////////////////////////////////////
ProtoJs::ProtoJs()
{
  cout << "ProtoJs ctr " << this << endl;
}

/////////////////////////////////////////////////
ProtoJs::~ProtoJs()
{
  cout << "ProtoJs dtor " << this << endl;
}

//////////////////////////////////////////////////
Handle<Value> ProtoJs::Op0(const Arguments& args)
{
  HandleScope scope;
  // we expect one string argument
  unsigned int argc = args.Length();
  if ( (argc < 3)  || (argc > 3)  )
  {
    std::stringstream e; 
    e << "Wrong number of arguments (expected " << 1 << " but got " <<  argc << " instead)";
   
    ThrowException(v8::Exception::TypeError(String::New(e.str().c_str())));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsString())
  {

    ThrowException(v8::Exception::TypeError(
        String::New("Wrong argument type. Type String expected as argument 1.")));
    return scope.Close(Undefined());
  }

  if (!args[1]->IsString())
  {

    ThrowException(v8::Exception::TypeError(
        String::New("Wrong argument type. Type String expected as argument 2.")));
    return scope.Close(Undefined());
  }

  if (!args[2]->IsFunction())
  {
    ThrowException(v8::Exception::TypeError(
        String::New("Wrong argument type. Function  expected as argument 3.")));
    return scope.Close(Undefined());
  }

  String::Utf8Value sarg0(args[0]->ToString());
  String::Utf8Value sarg1(args[1]->ToString());

  Op0CallBackData* async = new Op0CallBackData();
  async->cb = v8::Persistent<v8::Function>::New(v8::Local<v8::Function>::Cast(args[2]));
  // async->args = args;
  async->config = *sarg0;
  async->stlPath= *sarg1;
  uv_work_t *req = new uv_work_t;
  req->data = (void*)async;

  uv_queue_work(
    uv_default_loop(),
    req,                          // work token
    Op0Async,                     // work function
    (uv_after_work_cb)Op0Return   // function to run when complete
  );
  return scope.Close(Undefined());
}

void ProtoJs::Op0Async(uv_work_t *req)
{
    cout << "ProtoJs::Op0Async..." << endl;
    Op0CallBackData *data = (Op0CallBackData *)req->data;

    string configStr = data->config;
    cout << "config: " << configStr << endl;

    string modelFile = data->stlPath;
    cout << "stl: " << modelFile << endl;

    string gcodeFile = "./out.gcode";
    cout << "output: " << gcodeFile << endl; 

//    sleep(5);
    Configuration config;
    try {
    	config.readFromString(configStr.c_str());
	GCoderConfig gcoderCfg;
	loadGCoderConfigFromFile(config, gcoderCfg);

        SlicerConfig slicerCfg;
        loadSlicerConfigFromFile(config, slicerCfg);
	const char* scad = NULL;

	Tomograph tomograph;
	Regions regions;
	std::vector<mgl::SliceData> slices;

	ProgressLog log;

	int firstSliceIdx = -1;
	int	lastSliceIdx = -1;

	miracleGrue(gcoderCfg, 
                slicerCfg,
                modelFile.c_str(),
		scad,
		gcodeFile.c_str(),
		firstSliceIdx,
		lastSliceIdx,
		tomograph,
		regions,
		slices,
		&log);

    	cout << "done" << endl;
    }
    catch(mgl::Exception &e)
    {
       cout<< "Error: " << e.error << endl;
    }

}

void ProtoJs::Op0Return(uv_work_t *req)
{
    cout << "ProtoJs::Op0Return!" << endl;
    HandleScope scope;

    // retreive async data
    JsCallbackData *data = (JsCallbackData *)req->data;
    // generate callabck argument list
    const unsigned int argc = 2;
    v8::Handle<v8::Value> argv[argc] = {
    v8::Local<Value>::New(Null()),    
        v8::Local<v8::Value>::New(v8::String::New("data?"))
    };

   v8::TryCatch try_catch;
   (*data->cb)->Call(v8::Context::GetCurrent()->Global(), argc, argv);
   if (try_catch.HasCaught())
      node::FatalException(try_catch);

   // clean up async stuff
   data->cb.Dispose();
   delete data;
   delete req;
}

////////////////////////////////////////////////
Handle<Value> ProtoJs::Op1(const Arguments& args)
{

  HandleScope scope;
  // we expect one string argument
  unsigned int argExpected = 1;
  unsigned int argc = args.Length();
  if ( (argc < argExpected)  || (argc > argExpected)  )
  {
    std::stringstream e;
    e << "Wrong number of arguments (expected " << argExpected << " but got " <<  argc << " instead)";
    ThrowException(v8::Exception::TypeError(String::New(e.str().c_str())));
    return scope.Close(Undefined());
  }
/*
  if (!args[0]->IsString())
  {

    ThrowException(Exception::TypeError(
        String::New("Wrong argument type. Type String expected as argument 1.")));
    return scope.Close(Undefined());
  }
*/
  String::Utf8Value sarg0(args[0]->ToString());
  std::string arg0(*sarg0);
  return scope.Close(String::New(arg0.c_str()));
}
