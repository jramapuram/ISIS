/* 
 * File:   COM.h
 * Author: jason
 *
 * Created on May 17, 2013, 10:47 AM
 */

#ifndef COM_H
#define	COM_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "Application.h"
#include "SingletonApplication.h"

using boost::asio::ip::tcp;
namespace isis{

class COM:public SingletonApplication<COM>,public Application{
    public:
        COM();
        ~COM();
        void Run();
    private:
        static const int SRVPORT=6969;
};

}
#endif	/* COM_H */

