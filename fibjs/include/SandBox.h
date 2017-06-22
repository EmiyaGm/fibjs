/*
 * SandBox.h
 *
 *  Created on: Oct 22, 2012
 *      Author: lion
 */

#include "ifs/SandBox.h"
#include "ifs/Stream.h"
#include "ifs/process.h"
#include "ifs/Worker.h"
#include <map>
#include <vector>

#ifndef SANDBOX_H_
#define SANDBOX_H_

namespace fibjs {

class SandBox : public SandBox_base {
public:
    SandBox();

public:
    // SandBox_base
    virtual result_t add(exlib::string id, v8::Local<v8::Value> mod);
    virtual result_t add(v8::Local<v8::Object> mods);
    virtual result_t addScript(exlib::string srcname, Buffer_base* script, v8::Local<v8::Value>& retVal);
    virtual result_t remove(exlib::string id);
    virtual result_t clone(obj_ptr<SandBox_base>& retVal);
    virtual result_t run(exlib::string fname, v8::Local<v8::Array> argv);
    virtual result_t resolve(exlib::string id, exlib::string base, exlib::string& retVal);
    virtual result_t require(exlib::string id, exlib::string base, v8::Local<v8::Value>& retVal);
    virtual result_t get_global(v8::Local<v8::Object>& retVal);

public:
    v8::Local<v8::Object> mods()
    {
        Isolate* isolate = holder();

        v8::Local<v8::Value> v = GetPrivate("_mods");
        v8::Local<v8::Object> o;

        if (!v->IsUndefined())
            o = v->ToObject();
        else {
            o = v8::Object::New(isolate->m_isolate);
            SetPrivate("_mods", o);
        }

        return o;
    }

public:
    class Context {
    public:
        Context(SandBox* sb, exlib::string id);
        static result_t repl(v8::Local<v8::Array> cmds, Stream_base* out);

    public:
        obj_ptr<SandBox> m_sb;
        v8::Local<v8::Value> m_id;
        v8::Local<v8::Function> m_fnRequest;
        v8::Local<v8::Function> m_fnRun;
    };

    class Scope {
    public:
        Scope(SandBox* sb)
        {
            if (sb->m_global) {
                v8::Local<v8::Object> _global = v8::Local<v8::Object>::Cast(sb->GetPrivate("_global"));
                _context = _global->CreationContext();
                _context->Enter();
            }
        }

        ~Scope()
        {
            if (!_context.IsEmpty())
                _context->Exit();
        }

    private:
        v8::Local<v8::Context> _context;
    };

public:
    class ExtLoader : public obj_base {
    public:
        ExtLoader(const char* ext)
            : m_ext(ext)
        {
        }

    public:
        virtual result_t run_script(Context* ctx, Buffer_base* src,
            exlib::string name, v8::Local<v8::Array> argv);
        virtual result_t run_main(Context* ctx, Buffer_base* src,
            exlib::string name, v8::Local<v8::Array> argv);
        virtual result_t run_worker(Context* ctx, Buffer_base* src,
            exlib::string name, Worker_base* master);
        virtual result_t run_module(Context* ctx, Buffer_base* src,
            exlib::string name, v8::Local<v8::Object> module, v8::Local<v8::Object> exports);

    public:
        virtual result_t run(Context* ctx, Buffer_base* src, exlib::string name,
            exlib::string arg_names, v8::Local<v8::Value>* args, int32_t args_count)
        {
            return CHECK_ERROR(Runtime::setError("SandBox: Invalid file format."));
        }

    public:
        exlib::string m_ext;
    };

public:
    void initRoot();
    void initRequire(v8::Local<v8::Function> func)
    {
        SetPrivate("require", func);
    }

    void initGlobal(v8::Local<v8::Object> global);

    void InstallModule(exlib::string fname, v8::Local<v8::Value> o);

    result_t installScript(exlib::string srcname, Buffer_base* script, v8::Local<v8::Value>& retVal);

    result_t loadFile(exlib::string fname, obj_ptr<Buffer_base>& data);

    result_t resolveFile(exlib::string& fname, obj_ptr<Buffer_base>& data,
        v8::Local<v8::Value>* retVal);
    result_t resolveId(exlib::string& id, obj_ptr<Buffer_base>& data, v8::Local<v8::Value>& retVal);
    result_t resolveModule(exlib::string base, exlib::string& id, obj_ptr<Buffer_base>& data,
        v8::Local<v8::Value>& retVal);
    result_t resolve(exlib::string base, exlib::string& id, obj_ptr<Buffer_base>& data,
        v8::Local<v8::Value>& retVal);

    result_t repl(v8::Local<v8::Array> cmds, Stream_base* out = NULL);

    result_t run_module(exlib::string id, exlib::string base, v8::Local<v8::Value>& retVal);
    result_t run_main(exlib::string fname, v8::Local<v8::Array> argv);
    result_t run_worker(exlib::string fname, Worker_base* worker);

    result_t get_loader(exlib::string fname, obj_ptr<ExtLoader>& retVal)
    {
        size_t cnt = m_loaders.size();

        for (size_t i = 0; i < cnt; i++) {
            obj_ptr<ExtLoader>& l = m_loaders[i];

            if ((fname.length() > l->m_ext.length())
                && !qstrcmp(fname.c_str() + fname.length() - l->m_ext.length(), l->m_ext.c_str())) {
                retVal = l;
                return 0;
            }
        }

        return CHECK_ERROR(Runtime::setError("SandBox: Invalid file format."));
    }

public:
    static const char* script_args;
    static const int32_t script_args_count;

    static const char* main_args;
    static const int32_t main_args_count;

    static const char* worker_args;
    static const int32_t worker_args_count;

    static const char* module_args;
    static const int32_t module_args_count;

public:
    std::vector<obj_ptr<ExtLoader>> m_loaders;
    bool m_global;
};

} /* namespace fibjs */
#endif /* SANDBOX_H_ */
