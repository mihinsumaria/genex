#include "GroupsEqualLength.hpp"

#include <vector>

#include <iostream> //debug
#include "TimeSeries.hpp"
#include "Group.hpp"
#include "Exception.hpp"

namespace genex {

GroupsEqualLength::~GroupsEqualLength()
{
  for (unsigned int i = 0; i < groups.size(); i++)
  {
    delete groups[i];
    groups[i] = NULL;
  }

  groups.clear();
}

Group* GroupsEqualLength::getGroup(int idx) const
{
  if (idx < 0 || idx >= this->length) {
    throw GenexException("Index is out of range");
  }
  return this->groups[idx];
}

int GroupsEqualLength::getNumberOfGroups(void) const
{
  return this->groups.size();
}

void GroupsEqualLength::genGroups(DistanceMetric* metric, data_t threshold)
{

  for (int start = 0; start < perSeq; start++)
  {
    for (int idx = 0; idx < dataset.getItemCount(); idx++)
    {
      TimeSeries query = dataset.getTimeSeries(idx, start, start+length);

      data_t bsf = threshold/2 + 0.01;
      int bsfIndex = -1;

      for (unsigned int i = 0; i < groups.size(); i++)
      {
        data_t dist = groups[i]->distance(query, metric, INF);
        if (dist < bsf)
        {
          bsf = dist;
          bsfIndex = i;
        }
      }

      if (bsf > threshold/2)
      {
        bsfIndex = groups.size();
        this->groups.push_back(new Group(dataset, length));
      }

      this->groups[bsfIndex]->addMember(idx, start);
    }
  }
  //if we care about order:
  //std::sort(groups.begin(), groups.end(), &_group_gt_op);
}

int GroupsEqualLength::getBestGroup(const TimeSeries& query, DistanceMetric* metric, data_t& out_dist, data_t dropout) const
{
  data_t bsfDist = dropout;
  int bsfIndex = -1;

  for (unsigned int i = 0; i < groups.size(); i++) {

      data_t dist = groups[i]->distance(query, metric, bsfDist);

      if ((dist < bsfDist) || (bsfDist == INF)) {
          bsfDist = dist;
          bsfIndex = i;
      }
  }
  out_dist = bsfDist;
  return bsfIndex;
}


}//genex