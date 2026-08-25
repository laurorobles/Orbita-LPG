import re
import glob

def minify_cpp(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    # Remove single-line comments (ignoring URLs or within strings if we were perfect, but simple regex is fine here)
    # Actually, we don't have http:// in the code, so a simple // regex is fine.
    content = re.sub(r'//.*', '', content)
    
    # Remove block comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    
    # Remove excessive blank lines
    content = re.sub(r'\n\s*\n', '\n', content)
    
    # Optional: compact trailing spaces
    content = re.sub(r'[ \t]+$', '', content, flags=re.MULTILINE)
    
    # Optional: compress `{ \n` or similar? Let's just stick to comments and blank lines to maintain some readability.

    with open(filepath, 'w') as f:
        f.write(content)

for f in glob.glob("Source/*.cpp") + glob.glob("Source/*.h"):
    minify_cpp(f)
