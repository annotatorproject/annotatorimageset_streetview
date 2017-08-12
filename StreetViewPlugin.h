// Copyright 2017 Annotator Team

#ifndef STREETVIEWPLUGIN_H
#define STREETVIEWPLUGIN_H

#include <AnnotatorLib/ImageSet/ImageSetPlugin.h>

class StreetViewPlugin : public AnnotatorLib::ImageSet::ImageSetPlugin {
 public:
  StreetViewPlugin();

  virtual const std::string name() override { return "streetview"; }
  virtual const std::string description() override {
    return "StreetView Image Set";
  }

  virtual std::shared_ptr<AnnotatorLib::ImageSet::AbstractImageSet> create(
      std::string path) override;
};

#endif  // STREETVIEWPLUGIN_H
