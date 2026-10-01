import xml.etree.ElementTree as ET

def add_to_vcxproj(vcxproj_path, cpp_file, h_file):
    ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')
    tree = ET.parse(vcxproj_path)
    root = tree.getroot()
    ns = {'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'}

    for item_group in root.findall('msbuild:ItemGroup', ns):
        if item_group.find('msbuild:ClCompile', ns) is not None:
            exists = False
            for cl in item_group.findall('msbuild:ClCompile', ns):
                if cl.get('Include') == cpp_file:
                    exists = True
                    break
            if not exists:
                new_cl = ET.Element('ClCompile')
                new_cl.set('Include', cpp_file)
                item_group.append(new_cl)
            break

    for item_group in root.findall('msbuild:ItemGroup', ns):
        if item_group.find('msbuild:ClInclude', ns) is not None:
            exists = False
            for cl in item_group.findall('msbuild:ClInclude', ns):
                if cl.get('Include') == h_file:
                    exists = True
                    break
            if not exists:
                new_cl = ET.Element('ClInclude')
                new_cl.set('Include', h_file)
                item_group.append(new_cl)
            break
            
    tree.write(vcxproj_path, encoding='utf-8', xml_declaration=True)

def add_to_filters(filters_path, cpp_file, h_file):
    ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')
    tree = ET.parse(filters_path)
    root = tree.getroot()
    ns = {'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'}

    for item_group in root.findall('msbuild:ItemGroup', ns):
        if item_group.find('msbuild:ClCompile', ns) is not None:
            exists = False
            for cl in item_group.findall('msbuild:ClCompile', ns):
                if cl.get('Include') == cpp_file:
                    exists = True
                    break
            if not exists:
                new_cl = ET.Element('ClCompile')
                new_cl.set('Include', cpp_file)
                filter_tag = ET.SubElement(new_cl, 'Filter')
                filter_tag.text = 'Source'
                item_group.append(new_cl)
            break

    for item_group in root.findall('msbuild:ItemGroup', ns):
        if item_group.find('msbuild:ClInclude', ns) is not None:
            exists = False
            for cl in item_group.findall('msbuild:ClInclude', ns):
                if cl.get('Include') == h_file:
                    exists = True
                    break
            if not exists:
                new_cl = ET.Element('ClInclude')
                new_cl.set('Include', h_file)
                filter_tag = ET.SubElement(new_cl, 'Filter')
                filter_tag.text = 'Source'
                item_group.append(new_cl)
            break

    tree.write(filters_path, encoding='utf-8', xml_declaration=True)

add_to_vcxproj('DxLesson.vcxproj', 'Source\\NeedleTrap.cpp', 'Source\\NeedleTrap.h')
add_to_filters('DxLesson.vcxproj.filters', 'Source\\NeedleTrap.cpp', 'Source\\NeedleTrap.h')
