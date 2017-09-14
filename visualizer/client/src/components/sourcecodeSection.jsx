import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import SyntaxHighlighter from 'react-syntax-highlighter';
import { atomOneDark } from 'react-syntax-highlighter/dist/styles';

const styles = theme => ({
  highlighterRoot: {
    border: `1px solid ${theme.palette.primary[500]}`,
  },
});

/**
 * This class implement the logicfor the sourcecode highligher.
 * Based on the warning selected by the user the correspondent lines are highligted with
 * the appropriate color for the warning type
 */
class SourcecodeSection extends React.PureComponent {
  constructor() {
    super();
    this.state = {
      value: null,
    };
  }

  /**
   * based on the line number it gets if its bound to a warning and it return the
   * color of that warning as a background color of the line
   */
  highlightLine = (num) => {
    if (num === 4) {
      return { backgroundColor: 'green' };
    }
    return { backgroundColor: 'none' };
  }

  render() {
    const classes = this.props.classes;
    return (
      <SyntaxHighlighter
        language="cpp"
        showLineNumbers
        style={atomOneDark}
        className={classes.highlighterRoot}
        wrapLines
        lineStyle={this.highlightLine}
        lineNumberStyle={this.highlightLine}
      >
        {
          `#include <sanitizer/dfsan_interface.h>
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

int global_i1;  // global

int foo(int foo_arg_i1, int foo_arg_i2, char* foo_arg_cptr1) {
    // Taint propagation: from argument to local variable
    // Tainted: i1, f1, sb1, hb2, i2, i8, i4, global_i1, i6, foo_i1.1, foo_i1.2
    int foo_i1 = foo_arg_i1;
    return foo_i1;
}
            `
        }
      </SyntaxHighlighter>
    );
  }
}

SourcecodeSection.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(SourcecodeSection);
