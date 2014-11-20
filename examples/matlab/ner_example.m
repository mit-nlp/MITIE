% This example shows how to use the MITIE MATLAB API to perform named entity 
% recognition.  Note that you must compile the MATLAB API before you can
% use it.  To do this, execute the following commands:
%   cd examples/matlab
%   mkdir build
%   cd build
%   cmake -G "Visual Studio 10 Win64" ..
%   cmake --build . --config release --target install
% That will compile everything and copy the build outputs into the 
% examples/matlab folder.  You can then run this matlab script.  Note that
% you will need to change the argument to the -G option to match your
% compiler if you don't have Visual Studio 10.


% Load a text file as a cell array of words
tokens = tokenize_file('..\..\sample_text.txt')

% Now ask MITIE to identify all the entities in tokens.  Note that we need
% to supply a ner model file.  Additionally, the model file is loaded on
% the first call to extract_entities() and then kept forever.  So if you
% want to load a different model file within the same MATLAB instance you
% need to do "clear mex" to unload the MITIE mex files.  Then the next call
% to extract_entities() will load whatever model file you supply.
[entities,labels] = extract_entities(tokens, '../../MITIE-models/english/ner_model.dat')

disp(['num entities found: ' num2str(numel(entities))])

% Now print out all the entities, in each iteration, we print the entity
% label and then the text of the entity itself.  Note that entities{i}
% gives the range in tokens corresponding to the i-th entity.
for i = 1:numel(entities)
    disp([labels{i} {tokens{entities{i}}}])        
end

