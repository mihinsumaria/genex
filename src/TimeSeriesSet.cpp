#include "TimeSeriesSet.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include <boost/tokenizer.hpp>

#include "distance/Distance.hpp"
#include "Exception.hpp"

namespace genex {

TimeSeriesSet::~TimeSeriesSet()
{
  this->clearData();
}

void TimeSeriesSet::loadData(const std::string& filePath, int maxNumRow,
                             int startCol, const std::string& separators)
{
  this->clearData();

  std::ifstream f(filePath.c_str());
  if (!f.is_open())
  {
    f.close();
    throw GenexException(std::string("Cannot open ") + filePath);
  }

  if (maxNumRow <= 0) { maxNumRow = 999999999; }

  int length = -1;
  std::string line;
  boost::char_separator<char> sep(separators.c_str());
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

  this->itemCount = 0;
  for (int row = 0; row < maxNumRow; row++, this->itemCount++)
  {
    // Read and process line by line
    if (getline(f, line))
    {
      tokenizer tokens(line, sep);

      // Number of columns in the first line is assumed to be number of columns of
      // the whole dataset
      if (row == 0)
      {
        // If this is the first row, set length of each row to length of this row
        length = std::distance(tokens.begin(), tokens.end());
      }
      else if (length != std::distance(tokens.begin(), tokens.end()))
      {
        f.close();
        this->clearData();
        throw GenexException("File contains time series with inconsistent lengths");
      }

      data.push_back(new data_t[length - startCol]);

      int col = 0;
      for (tokenizer::iterator tok_iter = tokens.begin();
           tok_iter != tokens.end(); tok_iter++, col++)
      {
        // Only read columns from startCol and after
        if (col >= startCol)
        {
          try
          {
            data[row][col - startCol] = (data_t)std::stod(*tok_iter);
          }
          catch (const std::invalid_argument& e)
          {
            f.close();
            this->clearData();
            throw GenexException("Dataset file contains unparsable text");
          }
          catch (const std::out_of_range& e)
          {
            f.close();
            this->clearData();
            throw GenexException("Values are out of range");
          }
        }
      }
    }
    else {
      break;
    }
  }

  if (f.bad())
  {
    f.close();
    this->clearData();
    throw GenexException("Error while reading file");
  }

  this->itemLength = length - startCol;
  this->filePath = filePath;

  f.close();
}

void TimeSeriesSet::clearData()
{
  for (int i = 0; i < this->itemCount; i++)
  {
    delete[] this->data[i];
    this->data[i] = nullptr;
  }
  this->data.clear();
  this->itemCount = 0;
  this->itemLength = 0;
}

TimeSeries TimeSeriesSet::getTimeSeries(int index, int start, int end) const
{
  if (index < 0 || index >= this->itemCount)
  {
    throw GenexException("Invalid time series index");
  }
  if (start < 0 || start >= end || end > this->itemLength)
  {
    throw GenexException("Invalid starting or ending position of a time series");
  }

  return TimeSeries(this->data[index], index, start, end);
}

TimeSeries TimeSeriesSet::getTimeSeries(int index) const
{
  return this->getTimeSeries(index, 0, this->itemLength);
}

std::pair<data_t, data_t> TimeSeriesSet::normalize(void)
{
  int length = this->getItemLength() * this->getItemCount();

  if ( !length )
  {
    throw GenexException("No data to normalize");
  }

  // TODO: use library instead. Why * instead of vector?
  // auto minAndMax = std::minmax(this->data);
  // data_t diff = *minAndMax.first - *minAndMax.second;
  data_t MIN = INF;
  data_t MAX = -INF;
  unsigned int i;

  // find min and max in 1.5 comparisons per element
  for (int ts = 0; ts < this->itemCount; ts++)
  {
    data_t x, y, z;

    // start at 0 if even, 1 if odd
    if ((i = (length % 2 != 0)))
    {
      if (data[ts][0] < MIN)
      {
        MIN = data[ts][0];
      }
      if (data[ts][0] > MAX)
      {
        MAX = data[ts][0];
      }
    }

    for (; i < this->itemLength - 1; i += 2)
    {
      x = data[ts][i];
      y = data[ts][i + 1];
      if ( x > y )
      {
        z = y;
        y = x;
        x = z;
      }
      if ( y > MAX )
      {
        MAX = y;
      }
      if ( x < MIN )
      {
        MIN = x;
      }
    }
  }

  data_t diff = MAX - MIN;

  // do not divide by zero
  if (diff == 0.0)
  {
    if (MAX != 0)
    {
      // zero out data (use memset instead...)
      for (int ts = 0; ts < this->itemCount; ts++)
      {
        for (i = 0; i < this->itemLength; i++)
        {
          data[ts][i] = 0;
        }
      }
    }
  }
  else
  {
    // normalize
    for (int ts = 0; ts < this->itemCount; ts++)
    {
      for (i = 0; i < this->itemLength; i++)
      {
        data[ts][i] = (data[ts][i] - MIN)/ diff;
      }
    }
  }

  return std::make_pair(MIN, MAX);
}

bool TimeSeriesSet::valid()
{
  return this->data.size() > 0;
}

data_t TimeSeriesSet::distanceBetween(int idx, int start, int length,
         const TimeSeries& other, const std::string& distance_name)
{
  const dist_t distance = getDistance(distance_name);
  return distance(this->getTimeSeries(idx, start, start + length), other, INF);
}

} // namespace genex
