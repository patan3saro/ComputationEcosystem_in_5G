from music21 import stream, note, tempo, metadata

# Crea uno stream per il brano
score = stream.Score()

# Aggiungi metadati
score.metadata = metadata.Metadata(
    title='Semplice Melodia',
    composer='Il Tuo Nome'
)

# Aggiungi un tempo
score.append(tempo.MetronomeMark(number=120))

# Crea una parte
part = stream.Part()

# Aggiungi note alla parte
notes = [
    note.Note('C4', quarterLength=1),
    note.Note('D4', quarterLength=1),
    note.Note('E4', quarterLength=1),
    note.Note('F4', quarterLength=1),
    note.Note('G4', quarterLength=1)
]

for n in notes:
    part.append(n)

# Aggiungi la parte allo spartito
score.append(part)

# Mostra lo spartito
score.show()
