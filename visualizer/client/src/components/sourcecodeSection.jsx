import React from 'react';
import PropTypes from 'prop-types';
import classnames from 'classnames';
import { withStyles } from 'material-ui/styles';
import SyntaxHighlighter from 'react-syntax-highlighter';
import { atomOneDark } from 'react-syntax-highlighter/dist/styles';

const styles = theme => ({
  highlighterRoot: {
    border: `1px solid ${theme.palette.primary[500]}`,
    marginTop: 70,
  },
  highlighterOverflowHidden: {
    overflowY: 'scroll',
    maxHeight: 300,
    marginTop: 0,
  },
});

/**
 * This class implement the logicfor the sourcecode highligher.
 * Based on the warning selected by the user the correspondent lines are highligted with
 * the appropriate color for the warning type
 */
class SourcecodeSection extends React.PureComponent {
  /**
   * based on the line number it gets if its bound to a warning and it return the
   * color of that warning as a background color of the line
   */
  highlightLine = (num) => {
    const lineProp = this.props.highlightedLines.filter(n => n.lineNo === num);
    let background = 'transparent';
    let textColor = 'inherit';
    if (lineProp.length !== 0) {
      background = lineProp[0].color;
      textColor = 'white';
    }
    return { backgroundColor: background, color: textColor };
  }

  render() {
    const classes = this.props.classes;
    const activeClasses = classnames(classes.highlighterRoot, {
      [classes.highlighterOverflowHidden]: this.props.overflowHidden,
    });

    return (
      <SyntaxHighlighter
        language="cpp"
        showLineNumbers
        style={atomOneDark}
        className={activeClasses}
        wrapLines
        lineStyle={this.highlightLine}
        lineNumberStyle={this.highlightLine}
      >
        {this.props.sourcecode}
      </SyntaxHighlighter>
    );
  }
}

SourcecodeSection.propTypes = {
  classes: PropTypes.object.isRequired,
  highlightedLines: PropTypes.arrayOf(PropTypes.object).isRequired,
  overflowHidden: PropTypes.bool,
  sourcecode: PropTypes.string.isRequired,
};

SourcecodeSection.defaultProps = {
  overflowHidden: false,
};

export default withStyles(styles)(SourcecodeSection);
