#!/usr/bin/env python3

import cmd
import os
import requests
import sys

class GPT (cmd.Cmd):
  intro = 'Welcome to the GPT-3 CLI. Type !help to list commands.\n'
  prompt = '>>> '

  default_url = 'https://api.openai.com/v1/chat/completions'
  default_model = 'gpt-3.5-turbo'
  default_temperature = 0.01

  initial_messages = [
  ]

  def __init__(self, url=None, model=None, temperature=None, apikey_filename='.apikey'):
    super().__init__()
    if os.path.isfile(apikey_filename):
      with open(apikey_filename, 'r') as f:
        self.api_key = f.read()
    else:
      exit('No API key found. Please create a file named .apikey in the same directory as this script and paste your API key in it.')
    self.url = url or self.default_url
    self.model = model or self.default_model
    self.temperature = temperature or self.default_temperature
    self.messages = self.initial_messages

  def do_user(self, message):
    self.messages.append({'content': message, 'role': 'user'})

  def do_bot(self, message):
    self.messages.append({'content': message, 'role': 'assistant'})

  def do_sys(self, message):
    self.messages.append({'content': message, 'role': 'system'})

  def do_send(self, message):
    if message:
      self.messages.append({'content': message, 'role': 'user'})
    headers = {
      'Content-Type': 'application/json',
      'Authorization': 'Bearer ' + self.api_key
    }
    data = {
      'model': self.model,
      'messages': self.messages,
      'temperature': self.temperature
    }
    try:
      response = requests.post(self.url, headers=headers, json=data)
      if response.status_code == 200:
        if len(response.json()['choices']) > 0:
          self.messages.append({'content': response.json()['choices'][0]['message']['content'], 'role': 'assistant'})
          print(response.json()['choices'][0]['message']['content'])
        else:
          print('Error: No response from assistant')
      else:
        print('Error: ' + response.text)
    except Exception as e:
      print('Error: ' + str(e))

  def do_reset(self, arg):
    self.messages = self.initial_messages

  def do_debug(self, arg):
    print(self.messages)

  def do_exit(self, arg):
    return True

  def do_help(self, arg):
    print('Commands:')
    print('  <message>         send a message as the user')
    print('  !user <message>   add a message as the user')
    print('  !bot <message>    add a message as the bot')
    print('  !sys <message>    add a message as the system')
    print('  !reset            reset the conversation history')
    print('  !exit             exit the program')
    print('  !help             show this help message')

  def precmd(self, line):
    line = line.strip()
    if line == '?': return 'help'
    if line.startswith('!'):
      if (cmd := line[1:].split(' ')[0]) in ['user', 'bot', 'sys', 'reset', 'exit', 'help', 'debug']:
        return cmd + ' ' + line[len(cmd) + 2:].strip()
      else:
        return line
    return 'send ' + line.strip()

  def emptyline(self):
    pass

  def complete(self, text, state):
    return None

if __name__ == '__main__':
  gpt = GPT()
  if len(sys.argv) > 1:
    line = ' '.join(sys.argv[1:])
    line = gpt.precmd(line)
    gpt.onecmd(line)
  gpt.cmdloop()