"""Check the implemented media-library boundaries without external dependencies."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
MEDIA = {'MediaEngine', 'MediaFileSystems', 'MediaAnalysis'}
COMPONENTS = MEDIA | {'App', 'Infrastructure', 'Platform', 'Translations'}
# Adjacent components may exchange through contracts in either direction.
NEIGHBOURS = {
    'App': {'MediaEngine'},
    'MediaEngine': {'MediaFileSystems'},
    'MediaFileSystems': {'MediaEngine', 'MediaAnalysis'},
    'MediaAnalysis': {'MediaFileSystems'},
}
GUI_OR_HOST = re.compile(r'^(?:Q(?:Widget|Window|Dialog|Application|Painter|Screen|GuiApplication|OpenGLWidget)|Qt(?:Widgets|Gui)|windows\.h|dwmapi\.h|d3d\w*\.h|X11/|AppKit/)')
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"\n]+)[>"]', re.M)


def inspect():
    failures = []
    counts = {}
    edges = set()
    for component in sorted(COMPONENTS):
        base = ROOT / 'src' / component
        files = sorted(p for p in base.rglob('*') if p.suffix in {'.cpp', '.h'})
        counts[component] = len(files)
        for path in files:
            text = path.read_text(encoding='utf-8-sig')
            text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
            relative = path.relative_to(ROOT).as_posix()
            for include in INCLUDE.findall(text):
                if component in MEDIA and GUI_OR_HOST.match(include):
                    failures.append(f'{relative}: GUI/host include {include}')
                target = None
                public = False
                parts = include.replace('\\', '/').split('/')
                if len(parts) > 2 and parts[0] == 'RetroMediaLab' and parts[1] in COMPONENTS:
                    target = parts[1]
                    resolved = ROOT / 'src' / target / 'include' / include
                    public = True
                    if not resolved.is_file():
                        failures.append(f'{relative}: missing public header {include}')
                else:
                    resolved = (path.parent / include).resolve()
                    try:
                        local = resolved.relative_to(ROOT / 'src').parts
                    except ValueError:
                        local = ()
                    if local and local[0] in COMPONENTS:
                        target = local[0]
                        public = len(local) > 1 and local[1] == 'include'
                if target is None or target == component:
                    continue
                edges.add((component, target))
                if not public:
                    failures.append(f'{relative}: private cross-component include {include}')
                if target in MEDIA and component in NEIGHBOURS and target not in NEIGHBOURS[component]:
                    failures.append(f'{relative}: forbidden media access {component} -> {target}')
                if component in MEDIA and target in {'App', 'Platform', 'Infrastructure'}:
                    failures.append(f'{relative}: media implementation depends on {target}')
                if 'include' in path.relative_to(base).parts and target in MEDIA:
                    failures.append(f'{relative}: public contract exposes adjacent library {target}')

    links = []
    for component in sorted(COMPONENTS):
        path = ROOT / 'src' / component / 'CMakeLists.txt'
        if not path.is_file():
            continue
        cmake = re.sub(r'#[^\n]*', '', path.read_text(encoding='utf-8-sig'))
        for body in re.findall(r'target_link_libraries\s*\((.*?)\)', cmake, re.S):
            tokens = body.split()
            if not tokens:
                continue
            owner = tokens[0]
            visibility = 'unspecified'
            for token in tokens[1:]:
                if token in {'PUBLIC', 'PRIVATE', 'INTERFACE'}:
                    visibility = token
                elif token.startswith('RetroMediaLab.'):
                    target = token.removeprefix('RetroMediaLab.').split('.')[0]
                    links.append((owner, token, visibility))
                    if target in MEDIA and component in NEIGHBOURS and target not in NEIGHBOURS[component]:
                        failures.append(f'{path.relative_to(ROOT)}: forbidden link {owner} -> {token}')
                    if component in MEDIA and target in MEDIA and visibility != 'PRIVATE':
                        failures.append(f'{path.relative_to(ROOT)}: exposed implementation link {owner} -> {token}')
                    if component in MEDIA and target in {'App', 'Platform', 'Infrastructure'}:
                        failures.append(f'{path.relative_to(ROOT)}: media target depends on {token}')
        if component in MEDIA:
            includes = re.findall(r'target_include_directories\s*\((.*?)\)', cmake, re.S)
            for body in includes:
                if re.search(r'\b(?:PUBLIC|INTERFACE)\b', body) and '/src' in body:
                    failures.append(f'{path.relative_to(ROOT)}: private source directory exposed')
    return counts, sorted(edges), sorted(set(links)), sorted(set(failures))


if __name__ == '__main__':
    counts, edges, links, failures = inspect()
    for name, count in counts.items():
        print(f'{name}: {count} C++ files checked')
    for source, target in edges:
        print(f'include: {source} -> {target}')
    for source, target, visibility in links:
        print(f'link: {source} -> {target} ({visibility})')
    for failure in failures:
        print(f'FAIL: {failure}')
    print(f'{"FAIL" if failures else "PASS"}: {sum(counts.values())} files, {len(failures)} boundary violations')
    sys.exit(bool(failures))
