#include "GeneralizedTerms/GeneralizedTermNetwork.h"


std::string delayedresharing::NodeText(delayedresharing::Symbol* node)
{
  std::string output = " [label=\""+ node->toSymbol()+"\",xlabel=\"";
  int i = 0;
  for(auto conversion : node->conversions)
  {
    output += "[ "+ delayedresharing::SharingModeName(std::get<0>(conversion))+"->"+ delayedresharing::SharingModeName(std::get<1>(conversion))+" ]";

    if(i<(node->conversions.size()-1))
    {
      output += ", ";
    }
    i++;
  } 
  return output + "\"];";
}