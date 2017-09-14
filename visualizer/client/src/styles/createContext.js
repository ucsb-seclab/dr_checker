// @flow

import { create } from 'jss';
import preset from 'jss-preset-default';
import { SheetsRegistry } from 'react-jss/lib/jss';
import { createMuiTheme } from 'material-ui/styles';
import { purple, green } from 'material-ui/colors';
import createGenerateClassName from 'material-ui/styles/createGenerateClassName';

const primaryColor = green;
const secondaryColor = purple;

const theme = createMuiTheme({
  palette: {
    primary: primaryColor,
    secondary: secondaryColor,
    type: 'dark',
  },
  overrides: {
    MuiDivider: {
      default: {
        backgroundColor: primaryColor[500],
      },
    },
  },
});

// Configure JSS
const jss = create(preset());
jss.options.createGenerateClassName = createGenerateClassName;

export const sheetsManager = new Map();

function createContext() {
  return {
    jss,
    theme,
    // This is needed in order to deduplicate the injection of CSS in the page.
    sheetsManager,
    // This is needed in order to inject the critical CSS.
    sheetsRegistry: new SheetsRegistry(),
  };
}

export default createContext;
