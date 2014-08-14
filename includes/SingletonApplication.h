#ifndef SINGLETONAPPLICATION_H
#define SINGLETONAPPLICATION_H

namespace isis{

template <class Derived>
class SingletonApplication{

public:
    static Derived* instance(){
        static Derived s_Application;
        return &s_Application;
    }

protected:
    SingletonApplication(){//:NULLWORK(boost::in_place(boost::ref(service))) {}

    }

    virtual ~SingletonApplication(){

    }

private:

    SingletonApplication(SingletonApplication const&);
    SingletonApplication& operator=(SingletonApplication const&);

};

} //namespace

#endif // APPLICATION_H
