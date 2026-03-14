

#ifndef preprocessor_h
#define preprocessor_h

#include "segments.h"
#include <istream>
#include <sstream>
#include <fstream>
#include <list>

namespace r_comp {

class RepliMacro;
class RepliCondition;
class RepliStruct {
public:
  static std::unordered_map<std::string, RepliMacro *> RepliMacros_;
  static std::unordered_map<std::string, int32> Counters_;
  static std::list<RepliCondition *> Conditions_;
  static uint32 GlobalLine_;
  static std::vector<std::string> LoadedFilePaths_;

  enum Type { Root, Structure, Set, Atom, Directive, Condition, Development };
  Type type_;
  std::string cmd_;
  std::string tail_;
  std::string label_;
  std::string error_;
  uint32 line_;
  std::list<RepliStruct *> args_;
  RepliStruct *parent_;
  std::string filePath_;

  RepliStruct(RepliStruct::Type type);
  ~RepliStruct();

  void reset(); // remove rags that are objects.

  uint32 getIndent(std::istream *stream);
  int32 parse(std::istream *stream, const std::string& file_path, uint32 &cur_indent, uint32 &prev_indent, int32 param_expect = 0);
  bool parseDirective(std::istream *stream, const std::string& file_path, uint32 &cur_indent, uint32 &prev_indent);
  int32 process();

  RepliStruct *findAtom(const std::string &name);

  /**
   * Load the Replicode file from the filename and call parse, which will store the
   * filename in any !load directives for later use.
   * \param filename The file path which is already combined with the directory of the code with 
   * the !load directive.
   * \return The parsed code.
   */
  RepliStruct *loadReplicodeFile(const std::string &filename);

  /**
   * Search RepliStruct::LoadedFilePaths_ to check if the filePath is already loaded. This checks
   * for equivalent file paths.So, for example, "Test/file.replicode" will match with
   * "/work/AERA/Test/file.replicode" and "Test/../Test/file.replicode" if they all
   * refer to the same file.
   * \param file_path The file path to check.
   * \return True if an equivalend file path is already loaded, otherwise false.
   */
  static bool isFileLoaded(const std::string& file_path);

  RepliStruct *clone() const;
  std::string print() const;
  std::string printError() const;

  friend std::ostream& operator<<(std::ostream &os, const RepliStruct &structure);
  friend std::ostream& operator<<(std::ostream &os, RepliStruct *structure);
};

class RepliMacro {
public:
  std::string name_;
  RepliStruct *src_;
  RepliStruct *dest_;
  std::string error_;

  RepliMacro(const std::string &name, RepliStruct *src, RepliStruct *dest);
  ~RepliMacro();

  uint32 argCount();
  RepliStruct *expandMacro(RepliStruct *old_struct);
};

class RepliCondition {
public:
  std::string name_;
  bool reversed_;

  RepliCondition(const std::string &name, bool reversed);
  ~RepliCondition();
  void reverse();
  bool isActive(std::unordered_map<std::string, RepliMacro*> &repli_macros, std::unordered_map<std::string, int32> &counters);
};

class dll_export Preprocessor {
private:
  typedef enum {
    T_CLASS = 0,
    T_SYS_CLASS = 1,
    T_SET = 2
  }ClassType;
  Metadata *metadata_;
  uint16 class_opcode_; // shared with sys_classes_
  std::unordered_map<std::string, RepliStruct *> template_classes_;
  void instantiateClass(RepliStruct *tpl_class, std::list<RepliStruct *> &tpl_args, std::string &instantiated_class_name);
  bool isSet(std::string class_name);
  bool isTemplateClass(RepliStruct *s);
  void getMember(std::vector<StructureMember> &members, RepliStruct *m, std::list<RepliStruct *> &tpl_args, bool instantiate);
  void getMembers(RepliStruct *s, std::vector<StructureMember> &members, std::list<RepliStruct *> &tpl_args, bool instantiate);
  ReturnType getReturnType(RepliStruct *s);
  void initialize(Metadata *metadata); // init definition_segment
public:
  RepliStruct *root_;

  Preprocessor();
  ~Preprocessor();
  bool process(std::istream *stream, // if an ifstream, stream must be open.
    const std::string& filePath, // the file path of the ifstream.
    std::ostringstream *outstream, // output stream=input stream where macros are expanded.
    std::string &error, // set when function fails, e.g. returns false.
    Metadata *metadata = NULL); // process will fill class_image, or use the exiting one if NULL.
};
}


#endif
