/**
 * Created by wjy50 on 18-4-24.
 */

#ifndef NEURAL_NETWORK_UNIQUEPOINTEREXT_H
#define NEURAL_NETWORK_UNIQUEPOINTEREXT_H

#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    static_assert(!std::is_array<T>::value, "T must not be array");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
};

template<typename T, typename SizeT>
std::unique_ptr<T> make_unique_array(SizeT size)
{
    static_assert(std::is_array<T>::value&& std::extent<T>::value==0,"T must be dynamic array");
    using U=typename std::remove_extent<T>::type;
    return (std::unique_ptr<T>(new U[size]));
}

#endif //NEURAL_NETWORK_UNIQUEPOINTEREXT_H
