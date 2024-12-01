# Green Space mobile app (react-native + expo)
Reference: https://docs.expo.dev/tutorial/create-your-first-app/

<br>
<br>
<br>

# Sqlite3 
Reference: https://www.geeksforgeeks.org/python-sqlite/

1. Connecting to the database

```
import sqlite3

# Connect to an SQLite database (or create it if it doesn't exist)
conn = sqlite3.connect('example.db')

# Verify the connection
print(conn.total_changes) # Output: 0
```

2. Creating a Table

```
# Create a cursor object
cursor = conn.cursor()

# Create table
cursor.execute('''CREATE TABLE IF NOT EXISTS stocks
(date text, trans text, symbol text, qty real, price real)''')
```

3. Inserting Data

```
# Insert a row of data
cursor.execute("INSERT INTO stocks VALUES ('2006-01-05','BUY','RHAT',100,35.14)")

# Save (commit) the changes
conn.commit()
```

4. Querying Data

```
# Query the data
cursor.execute("SELECT * FROM stocks")
rows = cursor.fetchall()

# Print the results
for row in rows:
print(row)
```

5. Updating Data
```
# Update data
cursor.execute("UPDATE stocks SET qty = 200 WHERE symbol = 'RHAT'")

# Commit the changes
conn.commit()
```

6. Deleting Data

```
# Delete data
cursor.execute("DELETE FROM stocks WHERE symbol = 'RHAT'")

# Commit the changes
conn.commit()
```

7. Closing the connection

```
# Close the connection
conn.close()
```