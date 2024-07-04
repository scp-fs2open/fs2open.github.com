Here is the nightly for ${date} - Revision ${revision}

% if not success:
[b][color=red]At least one of the nightly builds failed![/color][/b]
% endif
% for file in files:


Group: ${file.group}
[url=${file.url}]${file.name}[/url] \
    %for mirror in file.mirrors:
        %if loop.first:
(\
        %endif
[url=${mirror}]Mirror[/url]\
        %if loop.last:
)\
        %else:
, \
        %endif
    %endfor

% endfor

[code]
${log}
[/code]
