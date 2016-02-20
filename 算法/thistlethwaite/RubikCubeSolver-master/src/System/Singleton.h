#ifndef RUBIK_SINGLETON_H
#define RUBIK_SINGLETON_H

#include <assert.h>

template<class T>
class Singleton
{
    public:
        static T* instance()
        {
            if (m_instance == nullptr)
                m_instance = new T();

            assert(m_instance);

            return m_instance;
        }

    private:

        static T* m_instance;
};

template<class T> T* Singleton<T>::m_instance = nullptr;

#endif
