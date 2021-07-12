# Setting up your project

Currently build operations are handled by the vendor IDE for the microcontroller you are using for your project. These IDEs are pre-packaged with the necessary toolchains, allow for detailed peripheral and pin configuration, and handle automatic file generation for makefiles, linker scripts, assembly preprocessor instructions, etc. that match the MCU or development board chosen.

*NOTE*: The following instructions cover STMicro and NXP projects, but the instructions will be similar for other Eclipse-based IDEs

## STM32CubeIDE setup

When creating a new project, choose a new STM32 Project from the File->New menu. A new window will pop up for selecting a target. After doing so and clicking Next, a new popup will prompt for a project name. Within this window be sure to select C++ as the Targeted Language. Click Finish after this.

![Stm32Setup](./DocsSource/Stm32Setup.png)

**NOTE: After the project has been created, rename the Core->Src->main.c to main.cpp**

The MCU can now be customized with the Device Configurator within the IDE and the corresponding initialization code auto-generated. 

## NXP setup

From File->New select New C/C++ Project, which will bring up a target selection window. After choosing a target and clicking Next, the next window prompts for a project name. Within this window be sure to select C++ Project under the Project Type pane.

![NxpSetup](./DocsSource/NxpSetup.png)

After project creation the MCU's pins, peripherals, and clocks can be customized within the IDE and corresponding code auto-generated.

---
# Normal Syncroness project development - integrating CEF into a project

CEF is not used on its own, instead being pulled in to embedded projects as a module or library. For normal project work, simply clone the CEF repository locally and copy the resulting directory into your project's directory tree:
```
git clone http://bitbucket.syncroness.com:7990/scm/cef/cef.git
```

## Syncroness Internal CEF development - repository configuration

For actual CEF development, a target platform is desired for debug and test purposes - for this, the CEF repository is added to a parent repository as a submodule where CEF development can continue within an embedded context. From within a target (STM, NXP, etc) project repository/folder, the following commands will add CEF as a submodule and switch to a branch for further development

```
git submodule add cef http://bitbucket.syncroness.com:7990/scm/cef/cef.git Cef
git add .
git commit -m "adding submodule"
git push
cd Cef
git checkout  <cef_development_branch_name>
```

## Configuring builds

Once CEF is added to your embedded project it must also be added to the include paths for compiling and linking. Currently this must be done through the vendor IDE for your project, which auto-generates makefiles for build configurations.

### STM32

In STM32CubeIDE, right-click on the Cef directory in the Project Explorer's file tree and select "Add/remove include path". A window with both Debug and Release configurations selected will popup, select OK.

![Stm32Include](./DocsSource/Stm32Include.png)

### NXP/McuXpresso

Right-click on the project itsel in the Project Explorer pane and select Properties. Expand the C/C++ Build category and select Settings. Within the Tool Settings tab choose the MCU C++ Compiler->Includes selection and click the icon to add an include path, then enter the path to the CEF directory.

![NxpInclude](./DocSource/../DocsSource/NxpXpressoInclude.png)