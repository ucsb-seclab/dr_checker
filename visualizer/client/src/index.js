import React from 'react';
import { render } from 'react-dom';
import config from 'react-global-configuration';
import MainStructure from './pages/main';

// Application settings
config.set({
  // server endpoint
  endpoint: 'http://localhost:5000',
});

render(<MainStructure />, document.querySelector('#root')); // eslint-disable-line 
