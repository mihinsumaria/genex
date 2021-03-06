#ifndef IO_HPP
#define IO_HPP
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <iostream>

#include <fstream>

#include "TimeSeries.hpp"
#include "Exception.hpp"

namespace genex {

// Original for save and load with boost binary archive and zlib: 
// https://stackoverflow.com/questions/48034374/compressing-boost-binary-archive-with-zlib?rq=1

template<typename A>
void saveToFile(const A & obj, const std::string & fname) {
    std::ofstream ofs(fname, std::ios::binary);
    if (ofs)
    {
        boost::iostreams::filtering_ostreambuf filter_osbuf;

        // push the ofstream and the compressor
        filter_osbuf.push(
            boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
        filter_osbuf.push(ofs);

        // start the archive on the filtering buffer
        boost::archive::binary_oarchive boa(filter_osbuf);

        boa << obj;
    }
    else 
    {
        throw GenexException("Cannot open file");    
    }
}

template<typename A>
void loadFromFile(A & obj, const std::string & fname) {
    std::ifstream ifs(fname, std::ios::binary);
    if (ifs)
    {
        boost::iostreams::filtering_istreambuf filter_isbuf;

        // push the ifstream and the decompressor
        filter_isbuf.push(boost::iostreams::zlib_decompressor());
        filter_isbuf.push(ifs);

        // start the archive on the filtering buffer
        boost::archive::binary_iarchive bia(filter_isbuf);

        bia >> obj;
    }
    else 
    {
        throw GenexException("Cannot open file");
    }
}

} // namespace genex
#endif // IO_HPP