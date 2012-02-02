// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hyper_graph_action.h"
#include "optimizable_graph.h"
#include "g2o/stuff/macros.h"


#include <iostream>

namespace g2o {
  using namespace std;

  HyperGraphActionLibrary* HyperGraphActionLibrary::actionLibInstance = 0;

  HyperGraphAction::Parameters::~Parameters()
  {
  }

  HyperGraphAction::ParametersIteration::ParametersIteration(int iter) :
    HyperGraphAction::Parameters(),
    iteration(iter)
  {
  }

  HyperGraphAction::~HyperGraphAction()
  {
  }

  HyperGraphAction* HyperGraphAction::operator()(const HyperGraph*, Parameters*)
  {
    return 0;
  }

  HyperGraphElementAction::Parameters::~Parameters()
  {
  }

  HyperGraphElementAction::HyperGraphElementAction(const std::string& typeName_)
  {
    _typeName = typeName_;
  }

  void HyperGraphElementAction::setTypeName(const std::string& typeName_)
  {
    _typeName = typeName_;
  }


  HyperGraphElementAction* HyperGraphElementAction::operator()(HyperGraph::HyperGraphElement* , HyperGraphElementAction::Parameters* )
  {
    return 0;
  }
  
  HyperGraphElementAction* HyperGraphElementAction::operator()(const HyperGraph::HyperGraphElement* , HyperGraphElementAction::Parameters* )
  {
    return 0;
  }
  
  HyperGraphElementAction::~HyperGraphElementAction()
  {
  }

  HyperGraphElementActionCollection::HyperGraphElementActionCollection(const std::string& name_)
  {
    _name = name_;
  }

  HyperGraphElementActionCollection::~HyperGraphElementActionCollection()
  {
    for (ActionMap::iterator it = _actionMap.begin(); it != _actionMap.end(); ++it) {
      delete it->second;
    }
  }

  HyperGraphElementAction* HyperGraphElementActionCollection::operator()(HyperGraph::HyperGraphElement* element, HyperGraphElementAction::Parameters* params)
  {
    ActionMap::iterator it=_actionMap.find(typeid(*element).name());
    //cerr << typeid(*element).name() << endl;
    if (it==_actionMap.end())
      return 0;
    HyperGraphElementAction* action=it->second;
    return (*action)(element, params);
  }

  HyperGraphElementAction* HyperGraphElementActionCollection::operator()(const HyperGraph::HyperGraphElement* element, HyperGraphElementAction::Parameters* params)
  {
    ActionMap::iterator it=_actionMap.find(typeid(*element).name());
    if (it==_actionMap.end())
      return 0;
    HyperGraphElementAction* action=it->second;
    return (*action)(element, params);
  }

  bool HyperGraphElementActionCollection::registerAction(HyperGraphElementAction* action)
  {
    if (action->name()!=name()){
      cerr << __PRETTY_FUNCTION__  << ": invalid attempt to register an action in a collection with a different name " <<  name() << " " << action->name() << endl;
    }
    _actionMap.insert(make_pair ( action->typeName(), action) );
    return true;
  }

  HyperGraphActionLibrary::HyperGraphActionLibrary()
  {
  }

  HyperGraphActionLibrary* HyperGraphActionLibrary::instance()
  {
    if (actionLibInstance == 0) {
      actionLibInstance = new HyperGraphActionLibrary;
    }
    return actionLibInstance;
  }

  void HyperGraphActionLibrary::destroy()
  {
    delete actionLibInstance;
    actionLibInstance = 0;
  }

  HyperGraphActionLibrary::~HyperGraphActionLibrary()
  {
    for (HyperGraphElementAction::ActionMap::iterator it = _actionMap.begin(); it != _actionMap.end(); ++it) {
      delete it->second;
    }
  }
  
  HyperGraphElementAction* HyperGraphActionLibrary::actionByName(const std::string& name)
  {

    HyperGraphElementAction::ActionMap::iterator it=_actionMap.find(name);
    if (it!=_actionMap.end())
      return it->second;
    return 0;
  }

  bool HyperGraphActionLibrary::registerAction(HyperGraphElementAction* action)
  {
    HyperGraphElementAction* oldAction = actionByName(action->name());
    HyperGraphElementActionCollection* collection = 0;
    if (oldAction) {
      collection = dynamic_cast<HyperGraphElementActionCollection*>(oldAction);
      if (! collection) {
        cerr << __PRETTY_FUNCTION__ << ": fatal error, a collection is not at the first level in the library" << endl;
        return 0;
      }
    }
    if (! collection) {
      cerr << __PRETTY_FUNCTION__ << ": creating collection for \"" << action->name() << "\"" << endl;
      collection = new HyperGraphElementActionCollection(action->name());
      _actionMap.insert(make_pair(action->name(), collection));
    }
    return collection->registerAction(action);
  }
  

  WriteGnuplotAction::WriteGnuplotAction(const std::string& typeName_)
    : HyperGraphElementAction(typeName_)
  {
    _name="writeGnuplot";
  }

  DrawAction::Parameters::Parameters(){
  }

  DrawAction::DrawAction(const std::string& typeName_) 
    : HyperGraphElementAction(typeName_)
  {
    _name="draw";
    _previousParams = (Parameters*)0x42;
    refreshPropertyPtrs(0);
  }

  bool DrawAction::refreshPropertyPtrs(HyperGraphElementAction::Parameters* params_){
    if (_previousParams == params_)
      return false;
    DrawAction::Parameters* p=dynamic_cast<DrawAction::Parameters*>(params_);
    if (! p){
      _previousParams = 0;
      _show = 0;
      _showId = 0;
    } else {
      _previousParams = p;
      _show = p->makeProperty<BoolProperty>(_typeName+"::SHOW", true);
      _showId = p->makeProperty<BoolProperty>(_typeName+"::SHOW_ID", false);
    }
    return true;
  }

  void applyAction(HyperGraph* graph, HyperGraphElementAction* action, HyperGraphElementAction::Parameters* params, const std::string& typeName)
  {
    for (HyperGraph::VertexIDMap::iterator it=graph->vertices().begin(); 
        it!=graph->vertices().end(); ++it){
      if ( typeName.empty() || typeid(*it->second).name()==typeName){
        (*action)(it->second, params);
      }
    }
    for (HyperGraph::EdgeSet::iterator it=graph->edges().begin(); 
        it!=graph->edges().end(); ++it){
      if ( typeName.empty() || typeid(**it).name()==typeName)
        (*action)(*it, params);
    }
  }

} // end namespace
