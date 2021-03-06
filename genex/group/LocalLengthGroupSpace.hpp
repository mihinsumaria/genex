#ifndef LOCAL_LENGTH_GROUP_SPACE_H
#define LOCAL_LENGTH_GROUP_SPACE_H

#include <vector>
#include <functional>
#include <queue>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>

#include "TimeSeries.hpp"
#include "distance/Distance.hpp"
#include "group/Group.hpp"

using std::vector;

namespace genex {

typedef std::pair<const Group*, data_t> candidate_group_t;

class LocalLengthGroupSpace
{
public:
  /**
   *  @brief constructor for LocalLengthGroupSpace
   *
   *  this class contains all the groups of a given length
   *  for a TimeSeriesSet
   *
   *  @param dataset the dataset that the class creates groups for
   *  @param length the length of each time series in each group
   */
  LocalLengthGroupSpace(const TimeSeriesSet& dataset, int length);

  /**
   * @brief deconstructor for LocalLengthGroupSpace
   */
  ~LocalLengthGroupSpace();

  void reset();  

  /**
   *  @brief returns the number of groups inside this object
   *  @return the number of groups
   */
  int getNumberOfGroups(void) const;

   /**
   *  @return a group with given index
   */
  const Group* getGroup(int idx) const;
  
  void saveGroupsOld(std::ofstream &fout, bool groupSizeOnly) const;
  int loadGroupsOld(std::ifstream &fin);
  
  /**
   *  @brief generates all the groups for the timeseries of this length
   *
   *  @param pairwiseDistance the distance to use when computing the groups
   *  @param threshold the threshold to use when splitting into new groups
   *  @return number of generated groups
   */
  int generateGroups(const dist_t pairwiseDistance, data_t threshold);

  /**
   *  @brief gets the group closest to a query (measured from the centroid)
   *
   *  @param query the time series we're operating with
   *  @param metric the metric that determines the distance between ts
   *  @param dropout the dropout optimization param
   */
  candidate_group_t getBestGroup(const TimeSeries& query,
                                 const dist_t warpedDistance,
                                 data_t dropout) const;

  int interLevelKSim(const TimeSeries& query, 
                     const dist_t warpedDistance, 
                     vector<group_index_t> &bestSoFar, 
                     int k);
    
private:
  int length, subTimeSeriesCount;
  const TimeSeriesSet& dataset;
  vector<Group*> groups;
  vector<group_membership_t> memberMap;

  /*************************
   *  Start serialization
   *************************/
  friend class boost::serialization::access;
  template<class A>
  void save(A & ar, unsigned) const
  {
    ar << groups.size();
    for (auto g : groups) {
      ar << *g;
    }
  }

  template<class A>
  void load(A & ar, unsigned)
  {
    size_t numberOfGroups;
    ar >> numberOfGroups;
    
    for (int i = 0; i < numberOfGroups; i++)
    {
      auto g = new Group(i
                         , this->length
                         , this->subTimeSeriesCount
                         , this->dataset
                         , this->memberMap);
      ar >> *g;
      this->groups.push_back(g);
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
  
  /*************************
   *  End serialization
   *************************/
};

} // namespace genex

#endif //LOCAL_LENGTH_GROUP_SPACE_H
