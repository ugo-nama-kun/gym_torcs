/***************************************************************************

    file        : profiler.cpp
    author      : Henrik Enqvist IB (henqvist@abo.fi)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef PROFILER

#include <algorithm>
#include <iostream>
#include <iomanip>

#include <tgf.h>


//////////////////////////////////////////////////////////

ProfileInstance::ProfileInstance(const char * pname) {
  strncpy(this->name, pname, 255);
  this->calls = 0;
  this->addTime = 0.0;
  this->totalTime = 0.0;
  this->subTime = 0.0;
  this->openCalls = 0;
}

//////////////////////////////////////////////////////////

Profiler * Profiler::profiler = NULL;

Profiler::Profiler() {
  fStartTime = GfTimeClock();
  curProfile = NULL;
}

Profiler::~Profiler() {
  profiler = NULL;
}

Profiler * Profiler::getInstance() {
  if (profiler == NULL) {
    profiler = new Profiler();
  }
  return profiler;
}

void Profiler::startProfile(const char * name) {
  ProfileInstance * newProfile;
  // find or create profile instance
  std::vector<ProfileInstance *>::iterator iter = vecProfiles.begin();
  std::vector<ProfileInstance *>::iterator end = vecProfiles.end();
  for (; iter != end; ++iter) {
    if (strncmp((*iter)->name, name, 255) == 0) {
      newProfile = (*iter);
      goto found;
    }
  }
  newProfile = new ProfileInstance(name);
  vecProfiles.push_back(newProfile);
 found:
  newProfile->tmpStart = GfTimeClock();
  ++(newProfile->openCalls);
  ++(newProfile->calls);

  // add child and push current profile to stack
  if (curProfile != NULL) {
    curProfile->mapChildren.insert(std::pair<ProfileInstance*, void *>(newProfile, NULL));
    stkProfiles.push_back(curProfile);
    assert(stkProfiles.size() < 100);
  }
  curProfile = newProfile;
}

void Profiler::stopProfile() {
  assert(curProfile != NULL);
  assert(curProfile->openCalls > 0);

  double elapsed = 0.0;
  elapsed = GfTimeClock() - curProfile->tmpStart;
  // add a warning if time to short
  if (elapsed < 0.00001) {
    this->mapWarning.insert(std::pair<ProfileInstance *, void*>(curProfile, NULL));
  }
  curProfile->addTime += elapsed;
  // recursive call check
  --(curProfile->openCalls);
  if (curProfile->openCalls == 0) {
    curProfile->totalTime += elapsed;
  }

  // add child time to parent
  std::vector<ProfileInstance *>::reverse_iterator last = stkProfiles.rbegin();
  if (last != stkProfiles.rend()) {
    curProfile = (*last);
    curProfile->subTime += elapsed;
    stkProfiles.pop_back();
  } else {
    curProfile = NULL;
  }
}

void Profiler::stopActiveProfiles() {
  while (curProfile != NULL) {
    this->stopProfile();
  }
}

bool instanceLess(const ProfileInstance * p1, const ProfileInstance * p2) {
  return p1->totalTime < p2->totalTime;
}

#define TRUNC(a) ( (a) < 0.001 ? 0.0 : (a) )

void Profiler::printProfile() {
  double total = GfTimeClock() - fStartTime;
  std::cerr << "****************** PROFILE ***********************" << std::endl;
  std::cerr << "Total : " << total << std::endl;
  std::cerr << "| TOTAL % |  OWN %  | % of PA |  CALLS  |  NAME" << std::endl;
  std::cerr << "|---------|---------|---------|---------|---------" << std::endl;
  sort(vecProfiles.begin(), vecProfiles.end(), instanceLess);
  std::vector<ProfileInstance *>::reverse_iterator iter = vecProfiles.rbegin();
  std::vector<ProfileInstance *>::reverse_iterator end = vecProfiles.rend();
  for (; iter != end; ++iter) {
    std::cerr << "| " << std::setprecision(3) << std::setw(7) 
	 << TRUNC( (*iter)->totalTime * 100 / total )
	 << " | " << std::setprecision(3) << std::setw(7) 
	 << TRUNC( ((*iter)->addTime - 
		    (*iter)->subTime) * 100.0 / total )
	 << " |         | " 
	 << std::setw(7) << (*iter)->calls << " | " << (*iter)->name << std::endl;
     std::map<ProfileInstance *, void*>::iterator childiter = 
       (*iter)->mapChildren.begin();
     std::map<ProfileInstance *, void*>::iterator childend = 
       (*iter)->mapChildren.end();
     for (; childiter != childend; ++childiter) {
       std::cerr << "| " << std::setprecision(3) << std::setw(7) 
	    << TRUNC( (*childiter).first->totalTime * 100 / total )
	    << " | " << std::setprecision(3) << std::setw(7) 
	    << TRUNC( ((*childiter).first->addTime - (*childiter).first->subTime) * 100.0 / total )
	    << " | " << std::setprecision(3) << std::setw(7) 
	    << TRUNC( 100 * (*childiter).first->totalTime / (*iter)->totalTime )
	    << " | " << std::setw(7) << (*childiter).first->calls 
	    << " |   " << (*childiter).first->name << std::endl;
     }
     std::cerr << "----------------------------------------------------------" << std::endl;
  }
  std::map<ProfileInstance *, void*>::iterator warniter = mapWarning.begin();
  std::map<ProfileInstance *, void*>::iterator warnend = mapWarning.end();
  for (; warniter != warnend; ++warniter) {
    std::cerr << "Warning: Profile " << (*warniter).first->name << " may be to short" << std::endl;
  }
}

#endif /* PROFILER */
