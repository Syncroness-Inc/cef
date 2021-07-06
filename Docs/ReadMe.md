# This directory contains all the documentation files for the Common Embedded Software project.

The goal is to keep the documenation simple,  quick to learn, and easy to maintain.  As such, the documentation is presented in "bite sized" chunks, with the goal that no document should take more than 20 minutes to read/comprehend.  If more details is needed, then there is likely another series of documents , "theory of operation" in the module's source files, or the code itself.

The top level directory contains the following sub-directories:

* ProjectDocs
  * contain documents that relate to the entire project.
  * Example: Overall architecture, make system, coding standards
* DebugPort
  * documents about the DebugPort
  * may contain both python and embedded software aspects
* CommandHandling
  * Documents related to the command handling system
  * may contain both python and embedded software aspects
* PythonUtilities
* ContinuousIntegration

Each sub-directory should contain

* ReadMe.doc  Each read me file should have similiar structure and content as this file for consistency.
* sub directory DocsSource that contain the .md file graphical files, as well as the files to generate the graphical files (i.e. the "source" files)
* Optionally, further documentation related sub-directories

It is anticipated the documentation directory structure will change over time.  When changes occur, please take care to update the readme files appropriately.
