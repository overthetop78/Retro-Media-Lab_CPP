"""Refresh the Visual C++ browsing project; CMake remains the build definition."""
from pathlib import Path
import uuid
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
MSBUILD = "http://schemas.microsoft.com/developer/msbuild/2003"
PROJECT_GUID = "{3A6BA25D-958F-5DBF-A782-0FA936172DBF}"
ET.register_namespace("", MSBUILD)


def child(parent, name, text=None, **attributes):
    node = ET.SubElement(parent, "{" + MSBUILD + "}" + name, attributes)
    if text is not None:
        node.text = text
    return node


def write(path, document):
    ET.indent(document, space="  ")
    data = ET.tostring(document, encoding="unicode", xml_declaration=True) + "\n"
    if not path.exists() or path.read_text(encoding="utf-8") != data:
        path.write_text(data, encoding="utf-8")


def main():
    project = ET.Element("{" + MSBUILD + "}Project", DefaultTargets="Build", ToolsVersion="Current")
    configurations = child(project, "ItemGroup", Label="ProjectConfigurations")
    for configuration in ("Debug", "Release"):
        item = child(configurations, "ProjectConfiguration", Include=configuration + "|x64")
        child(item, "Configuration", configuration)
        child(item, "Platform", "x64")
    globals_ = child(project, "PropertyGroup", Label="Globals")
    for key, value in {"ProjectGuid": PROJECT_GUID, "Keyword": "MakeFileProj", "RootNamespace": "RetroMediaLab", "WindowsTargetPlatformVersion": "10.0"}.items():
        child(globals_, key, value)
    child(project, "Import", Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props")
    configuration = child(project, "PropertyGroup", Label="Configuration")
    child(configuration, "ConfigurationType", "Makefile")
    child(configuration, "PlatformToolset", "v143")
    child(project, "Import", Project="$(VCTargetsPath)\\Microsoft.Cpp.props")
    settings = child(project, "PropertyGroup")
    child(settings, "QtRoot", "$(RETROMEDIALAB_QT_DIR)", Condition="'$(QtRoot)' == ''")
    child(settings, "QtRoot", "C:\\Qt\\6.12.0\\msvc2022_64", Condition="'$(QtRoot)' == ''")
    child(settings, "CMakeExecutable", "$(VSInstallDir)Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe", Condition="'$(CMakeExecutable)' == ''")
    build = 'powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(ProjectDir)scripts\\local-building\\build.ps1" -Configuration "$(Configuration)" -QtRoot "$(QtRoot)"'
    clean = '"$(CMakeExecutable)" --build "$(ProjectDir)build\\$(Configuration)" --target clean'
    child(settings, "NMakeBuildCommandLine", build)
    child(settings, "NMakeReBuildCommandLine", clean + " &amp;&amp; " + build)
    # ElementTree escapes the ampersands in command lines itself.
    settings.find("{" + MSBUILD + "}NMakeReBuildCommandLine").text = clean + " && " + build
    child(settings, "NMakeCleanCommandLine", clean)
    output = "$(ProjectDir)build\\$(Configuration)\\Retro Media Lab\\retromedialab.exe"
    child(settings, "NMakeOutput", output)
    child(settings, "LocalDebuggerCommand", output)
    child(settings, "LocalDebuggerWorkingDirectory", "$(ProjectDir)build\\$(Configuration)\\Retro Media Lab")
    child(settings, "DebuggerFlavor", "WindowsLocalDebugger")
    child(settings, "OutDir", "$(ProjectDir)build\\ide\\$(Configuration)\\")
    child(settings, "IntDir", "$(OutDir)")
    child(settings, "NMakePreprocessorDefinitions", "UNICODE;_UNICODE;QT_CORE_LIB;QT_GUI_LIB;QT_WIDGETS_LIB;QT_NETWORK_LIB;RETROMEDIALAB_APP_BUILD;RETROMEDIALAB_INFRASTRUCTURE_BUILD;RETROMEDIALAB_TRANSLATIONS_BUILD;RETROMEDIALAB_MEDIAANALYSIS_BUILD;RETROMEDIALAB_MEDIAFILESYSTEMS_BUILD;RETROMEDIALAB_MEDIAENGINE_BUILD;$(NMakePreprocessorDefinitions)")
    includes = ["src\\MediaEngine\\include", "src\\MediaFileSystems\\include", "src\\MediaAnalysis\\include", "src\\App", "src\\Infrastructure", "src\\Infrastructure\\include", "src\\Common", "third_party\\libarchive\\libarchive", "build\\$(Configuration)\\third_party\\libarchive", "src\\Translations\\include", "src\\Platform\\include", "build\\$(Configuration)\\generated"]
    includes = ["$(ProjectDir)" + item for item in includes]
    includes += ["$(QtRoot)\\include", "$(QtRoot)\\include\\QtCore", "$(QtRoot)\\include\\QtGui", "$(QtRoot)\\include\\QtWidgets", "$(QtRoot)\\include\\QtNetwork", "$(QtRoot)\\mkspecs\\win32-msvc", "$(NMakeIncludeSearchPath)"]
    child(settings, "NMakeIncludeSearchPath", ";".join(includes))
    child(settings, "NMakeAdditionalOptions", "/std:c++20 /utf-8 $(NMakeAdditionalOptions)")
    filters = ET.Element("{" + MSBUILD + "}Project", ToolsVersion="Current")
    filter_definitions = child(filters, "ItemGroup")
    files = sorted(path for folder in (ROOT / "src", ROOT / "scripts", ROOT / "third_party") for path in folder.rglob("*") if path.is_file() and "__pycache__" not in path.parts)
    files += [ROOT / name for name in ("CMakeLists.txt", "CMakePresets.json", "README.md")]
    source_items = child(project, "ItemGroup")
    filter_items = child(filters, "ItemGroup")
    defined = set()
    for path in files:
        relative = path.relative_to(ROOT)
        filter_name = str(relative.parent).replace("/", "\\")
        if filter_name == ".":
            filter_name = "Project"
        parts = filter_name.split("\\")
        for index in range(len(parts)):
            name = "\\".join(parts[:index + 1])
            if name not in defined:
                defined.add(name)
                definition = child(filter_definitions, "Filter", Include=name)
                child(definition, "UniqueIdentifier", "{" + str(uuid.uuid5(uuid.NAMESPACE_URL, "RetroMediaLab/" + name)).upper() + "}")
        kind = "ClCompile" if path.suffix in {".cpp", ".c"} else "ClInclude" if path.suffix == ".h" else "None"
        filename = str(relative).replace("/", "\\")
        child(source_items, kind, Include=filename)
        child(child(filter_items, kind, Include=filename), "Filter", filter_name)
    child(project, "Import", Project="$(VCTargetsPath)\\Microsoft.Cpp.targets")
    write(ROOT / "RetroMediaLab.vcxproj", project)
    write(ROOT / "RetroMediaLab.vcxproj.filters", filters)


if __name__ == "__main__":
    main()
