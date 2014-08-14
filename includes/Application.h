#ifndef APPLICATION_H
#define APPLICATION_H
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>

#include <string>
#include <cxxabi.h>
#include <unordered_map>
#include <utility>
#include <iterator>
#include <stdexcept>

#include "trace.h"
#include "Isis.h"

namespace isis{

typedef boost::signals2::signal<void ()> VOID_SIGNAL;
typedef boost::shared_ptr<VOID_SIGNAL> VOID_SIGNAL_ARRAY;
typedef std::unordered_map<std::string, VOID_SIGNAL_ARRAY > SIGNAL_MAP_TYPE;
typedef boost::function<void ()> VOID_FUNC;

//template <class Derived>
class Application{
public:

    std::shared_ptr<LogModule>             log() {
        if(!log_){
            log_.reset(Isis::logging->getModule(AppName));
        }
        return log_;
    }

    virtual void Run() = 0;

    void RunApp(){
        try{
            Run();
        }catch(const boost::thread_interrupted& ex){
            log()->error("Thread %s Has Been Interrupted!",AppName);
            Isis::isRunning = false;
            Isis::IsisThreads.interrupt_all();
        }catch(...){
            log()->error("Unknown Exception Detected In %s Thread !",AppName);
            Isis::isRunning=false;
        }
    }

    void setName(const std::string& Name){
        AppName=Name;
    }

    VOID_SIGNAL_ARRAY& getSignal(const std::string& key){
        return m_Signals.at(key);
    }

    void addSignal(const std::string& key){
        VOID_SIGNAL_ARRAY new_sig(new VOID_SIGNAL());
        { //scoped_lock
            boost::mutex::scoped_lock lock(m_SignalsLock);
            std::pair< SIGNAL_MAP_TYPE::iterator,bool > retVal=m_Signals.insert(std::make_pair(key, new_sig));
            if(retVal.second){
                log()->info("Successfully added signal: [%s], MapSize: [%d]",key,m_Signals.size());
            }else{
                log()->error("Signal [%s] was NOT added successuly!!",key);
            }
        }
    }

    void removeEvent(const std::string& eventName){
        try{
            { //scoped_lock
                boost::mutex::scoped_lock lock(m_SignalsLock);
                if(m_Signals.erase(eventName)==0){
                     log()->error("Error, no event named [%s] registerd",eventName);
                }else{
                    log()->info("Event [%s] deleted successfully",eventName);
                }
            }
        }catch(const std::exception& ex){
            log()->fatal("Exception at removing event :[%s]: ",eventName,ex.what());
        }
    }

    void raiseEvent(const std::string& eventName, bool ShouldWeDelete){
        try{
            { //scoped_lock
                boost::mutex::scoped_lock lock(m_SignalsLock);
                (*m_Signals.at(eventName))();
                log()->info("Raised Event: [%s]",eventName);
            }
            if(ShouldWeDelete) removeEvent(eventName);//has it's own mutex
        }catch(const std::out_of_range& oor){
            log()->fatal("OOR Exception at Event Raise: %s",oor.what());
            log()->fatal("Error, the map does not contain the desired event [exception block]!");
        }catch(const std::exception& ex){
            log()->fatal("General Exception at Event Raise: ",ex.what());
        }
    }

    template<class AppRef>\
    void registerEvent(AppRef& App, std::string EventName, VOID_FUNC cb){
        try{
            log()->info("Registering event: [%s]",EventName);
            VOID_SIGNAL_ARRAY Sig=App.getSignal(EventName);
            (*Sig).connect(cb);//boost::bind(&cb,this));
        }catch(const std::exception& ex){
            log()->fatal("Error during event register: %s",ex.what());
        }
    }

    std::string getName(){
        return AppName;
    }

protected:
    Application(std::string name) : AppName(name)
    {//:NULLWORK(boost::in_place(boost::ref(service))) {}
//        int status;
//        AppName = abi::__cxa_demangle(typeid(Derived).name(), 0, 0, &status);
//        if(status!=0) throw std::runtime_error("Error during class name demangling!");
        log()->info("INIT");
    }
    virtual ~Application(){
        //for(auto it : m_Signals){

        //}
        log()->info("Terminated");
    }

private:

    std::string                    AppName;
    boost::asio::io_service        service;
    bool                           isLogChannelSet;
    SIGNAL_MAP_TYPE                m_Signals;
    boost::mutex                   m_SignalsLock;
    std::shared_ptr<LogModule>     log_;
};



} //namespace

#endif // APPLICATION_H
