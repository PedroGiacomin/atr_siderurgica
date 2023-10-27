# Trabalho prático de ATR

## DevOps
- Colocar arquivos .h em /include
- Colocar os arquivos .cpp em /

Para desenvolver uma nova parte do código:
1. Cria uma nova branch com ``git branch <nome-da-branch>``
2. Muda pra essa branch com ``git checkout <nome-da-branch>`` 
3. Quando achar que já deve comitar as mudanças fazer:
    - ``git add .``
    - ``git commit -m "mensagem descritiva sobre o commit"``
    - ``git push -u origin <nome-da-branch>``
4. Deixar pra dar merge só quando estivermos em conjunto, fazendo
    - ``git checkout main``
    - ``git merge <nome-da-branch>``
    - ``git push -u origin main`` 